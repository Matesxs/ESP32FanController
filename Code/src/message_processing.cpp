//
// Created by Martin on 8. 7. 2025.
//

#include <queue>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <nvs_flash.h>
#include <esp_task_wdt.h>

#include <string_helpers.h>
#include <custom_debug.h>

#include "datastores.h"
#include "settings.h"
#include "pins.h"
#include "wifi_handler.h"

struct MessageData
{
  String message;
  Print* source;
};

static std::queue<MessageData> messageQueue;

extern uint8_t detectedTemperatureSensors;
extern TemperatureData temperatureData[MAXIMUM_TEMPERATURE_SENSORS];
extern float externalTemperature;

extern uint32_t currentFanSamplingTime;
extern uint32_t avgFanRPM[FAN_COUNT];
extern FanControlData fanControlData[PWM_CHANNELS_COUNT];

extern Preferences fanConfigs;
extern Preferences connectionSettings;

extern double pwmToPerc(uint32_t pwm);
extern uint32_t percToPwm(double percent);

JsonDocument generateFanData(const size_t fanChannel)
{
  JsonDocument fanDataDocument;

  for (size_t j = 0; j < FAN_COUNT; j++)
  {
    if (FAN_INDEX_TO_PWM_CHANNEL_INDEX[j] == fanChannel)
      fanDataDocument["rpm"][std::to_string(j + 1).c_str()] = avgFanRPM[j];
  }

  fanDataDocument["control"]["enabled"] = static_cast<bool>(fanControlData[fanChannel].settings.enabled);
  fanDataDocument["control"]["state"] = fanStateToString(fanControlData[fanChannel].state);
  fanDataDocument["control"]["temperature"] = fanControlData[fanChannel].pwmData.temperature;
  fanDataDocument["control"]["pwm"]["target"] = fanControlData[fanChannel].pwmData.targetPwm;
  fanDataDocument["control"]["pwm"]["current"] = fanControlData[fanChannel].pwmData.currentPwm;
  fanDataDocument["control"]["power"]["target"] = pwmToPerc(fanControlData[fanChannel].pwmData.targetPwm);
  fanDataDocument["control"]["power"]["current"] = pwmToPerc(fanControlData[fanChannel].pwmData.currentPwm);
  fanDataDocument["control"]["settings"]["start_power"] = fanControlData[fanChannel].settings.startPower;
  fanDataDocument["control"]["settings"]["start_temperature"] = fanControlData[fanChannel].settings.startTemperature;
  fanDataDocument["control"]["settings"]["min_power"] = fanControlData[fanChannel].settings.minimalPower;
  fanDataDocument["control"]["settings"]["stop_temperature"] = fanControlData[fanChannel].settings.minimalTemperature;
  fanDataDocument["control"]["settings"]["max_power"] = fanControlData[fanChannel].settings.maximalPower;
  fanDataDocument["control"]["settings"]["max_temperature"] = fanControlData[fanChannel].settings.maximalTemperature;
  fanDataDocument["control"]["settings"]["spool_up_time"] = fanControlData[fanChannel].settings.spoolUpTime;
  fanDataDocument["control"]["settings"]["spool_down_time"] = fanControlData[fanChannel].settings.spoolDownTime;
  fanDataDocument["control"]["settings"]["temp_source"] = fanControlData[fanChannel].settings.temperatureSourceSelect.to_string();

  return fanDataDocument;
}

void processMessage(MessageData& messageData)
{
  LOG_DEBUG("Processing message: %s", messageData.message.c_str());

  String command = getValue(messageData.message, ' ', 0);

  if (command == "PING")
  {
    messageData.source->println("PONG");
  }
  else if (command == "TEMPERATURES")
  {
    JsonDocument temperatureDataDocument;

    for (uint8_t i = 0; i < detectedTemperatureSensors; i++)
    {
      temperatureDataDocument[i]["working"] = temperatureData[i].valid;
      temperatureDataDocument[i]["temperature"] = temperatureData[i].temperature;
    }

    String formatedTemperatureData;
    serializeJsonPretty(temperatureDataDocument, formatedTemperatureData);
    messageData.source->println(formatedTemperatureData);
  }
  else if (command == "FAN_SPEED_SAMPLING_TIME")
  {
    messageData.source->printf("%.1fs\n", currentFanSamplingTime / 1000.0);
  }
  else if (command == "FAN_DATA")
  {
    // FAN_DATA 0

    JsonDocument fanDataDocument;

    String channelIndexString = getValue(messageData.message, ' ', 1);
    auto channelIndex = static_cast<size_t>(channelIndexString.toInt());
    if (channelIndexString.length() != 0)
    {
      if (channelIndex >= PWM_CHANNELS_COUNT)
        messageData.source->println("INVLD");
      else
        fanDataDocument = generateFanData(channelIndex);
    }
    else
    {
      for (size_t i = 0; i < PWM_CHANNELS_COUNT; i++)
        fanDataDocument[i] = generateFanData(i);
    }

    String formatedFanData;
    serializeJsonPretty(fanDataDocument, formatedFanData);
    formatedFanData += "\n";

    // Optimization for space saving
    messageData.source->write(formatedFanData.c_str(), formatedFanData.length());
  }
  else if (command == "SET_FAN_SPEED")
  {
    // SET_FAN_SPEED 0 0
    // SET_FAN_SPEED 0 25
    // SET_FAN_SPEED 0 50
    // SET_FAN_SPEED 0 100
    // SET_FAN_SPEED 0 auto

    String channelIndexString = getValue(messageData.message, ' ', 1);
    String fanSpeedString = getValue(messageData.message, ' ', 2);
    fanSpeedString.toLowerCase();

    auto channelIndex = static_cast<size_t>(channelIndexString.toInt());
    double fanSpeed = fanSpeedString.toDouble();

    if (channelIndexString.length() == 0 || fanSpeedString.length() == 0 ||
        channelIndex >= PWM_CHANNELS_COUNT ||
        fanSpeedString != "auto" && (isnan(fanSpeed) || fanSpeed < 0.0 || fanSpeed > 100.0))
    {
      messageData.source->println("INVLD");
    }
    else
    {
      std::lock_guard lock(fanControlData[channelIndex]._lock);

      if (fanSpeedString == "auto")
      {
        // If control is enabled change state of state machine and if not set PWM directly
        if (fanControlData[channelIndex].settings.enabled)
        {
          // If running then swich to running state else switch to stopped state
          if (fanControlData[channelIndex].pwmData.targetPwm > 0)
            fanControlData[channelIndex].state = FS_RUNNING;
          else
            fanControlData[channelIndex].state = FS_STOPPED;
        }
        else
        {
          fanControlData[channelIndex].pwmData.targetPwm = 0;
          fanControlData[channelIndex].state = FS_STOPPED;
        }
      }
      else
      {
        fanControlData[channelIndex].state = FS_FORCED;
        fanControlData[channelIndex].pwmData.targetPwm = percToPwm(fanSpeed);
      }

      messageData.source->println("OK");
    }
  }
  else if (command == "TEST_START_FAN_SPEED")
  {
    // TEST_START_FAN_SPEED 0

    String channelIndexString = getValue(messageData.message, ' ', 1);
    auto channelIndex = static_cast<size_t>(channelIndexString.toInt());

    if (channelIndexString.length() == 0 ||
        channelIndex >= PWM_CHANNELS_COUNT)
    {
      messageData.source->println("INVLD");
    }
    else
    {
      std::vector<size_t> fanIndexes;
      for (size_t i = 0; i < FAN_COUNT; i++)
      {
        if (FAN_INDEX_TO_PWM_CHANNEL_INDEX[i] == channelIndex) fanIndexes.push_back(i);
      }

      // Stop channel
      {
        std::lock_guard lock(fanControlData[channelIndex]._lock);
        fanControlData[channelIndex].state = FS_FORCED;
        fanControlData[channelIndex].pwmData.currentPwm = 0;
      }

      // Wait for stop
      messageData.source->println("Waiting for fans to stop");
      for (const auto& fanIndex: fanIndexes)
      {
        while (avgFanRPM[fanIndex] > 0)
        {
          vTaskDelay(pdMS_TO_TICKS(100));
          esp_task_wdt_reset();
        }
      }

      for (double perc = 0.5; perc <= 100.0; perc += 0.5) // NOLINT(*-flp30-c)
      {
        messageData.source->printf("Testing %.1f%% as start power setting for fan channel %d\n", perc, channelIndex);

        fanControlData[channelIndex].pwmData.currentPwm = percToPwm(perc);
        vTaskDelay(pdMS_TO_TICKS(100));
        esp_task_wdt_reset();

        messageData.source->println("Waiting for stabilization");
        uint32_t samplingWaitCycles = static_cast<uint32_t>(round((currentFanSamplingTime * FAN_SPEED_AVERAGE_SAMPLES + FAN_POWERON_BUMP_DURATION_MS + FAN_POWERON_RETRY_TIMEOUT_MS) / 1000.0)) + 2;
        for (uint32_t i = 0; i < samplingWaitCycles; i++)
        {
          vTaskDelay(pdMS_TO_TICKS(1000));
          esp_task_wdt_reset();
        }

        for (const auto& fanIndex: fanIndexes)
        {
          if (avgFanRPM[fanIndex] == 0) break;
          messageData.source->printf("FOUND %.1f\n", min(perc + 5.0, 100.0));

          if (fanControlData[channelIndex].settings.enabled)
            fanControlData[channelIndex].state = FS_RUNNING;
          else
          {
            fanControlData[channelIndex].pwmData.currentPwm = 0;
            fanControlData[channelIndex].state = FS_STOPPED;
          }
          return;
        }
      }

      if (fanControlData[channelIndex].settings.enabled)
        fanControlData[channelIndex].state = FS_RUNNING;
      else
      {
        fanControlData[channelIndex].pwmData.currentPwm = 0;
        fanControlData[channelIndex].state = FS_STOPPED;
      }

      messageData.source->println("NOT FOUND");
    }
  }
  else if (command == "SET_TEMP_CONTROL_PARAMETERS")
  {
    // SET_TEMP_CONTROL_PARAMETERS 0 25;50;20;20;40;100;10000;30000

    String channelIndexString = getValue(messageData.message, ' ', 1);
    String settingsString = getValue(messageData.message, ' ', 2);
    String startTemperatureString = getValue(settingsString, ';', 0);
    String startPowerString = getValue(settingsString, ';', 1);
    String stopTemperatureString = getValue(settingsString, ';', 2);
    String stopPowerString = getValue(settingsString, ';', 3);
    String maximumTemperatureString = getValue(settingsString, ';', 4);
    String maximumPowerString = getValue(settingsString, ';', 5);
    String spoolUpTimeString = getValue(settingsString, ';', 6);
    String spoolDownTimeString = getValue(settingsString, ';', 7);

    auto channelIndex = static_cast<size_t>(channelIndexString.toInt());
    float startTemperature = startTemperatureString.toFloat();
    double startPower = startPowerString.toDouble();
    float stopTemperature = stopTemperatureString.toFloat();
    double stopPower = stopPowerString.toDouble();
    float maximumTemperature = maximumTemperatureString.toFloat();
    double maximumPower = maximumPowerString.toDouble();
    auto spoolUpTime = max(static_cast<uint32_t>(10), static_cast<uint32_t>(spoolUpTimeString.toInt()));
    auto spoolDownTime = max(static_cast<uint32_t>(10), static_cast<uint32_t>(spoolDownTimeString.toInt()));

    if (channelIndexString.length() == 0 ||
        startTemperatureString.length() == 0 || startPowerString.length() == 0 ||
        stopTemperatureString.length() == 0 || stopPowerString.length() == 0 ||
        maximumTemperatureString.length() == 0 || maximumPowerString.length() == 0 ||
        spoolUpTimeString.length() == 0 || spoolDownTimeString.length() == 0 ||
        channelIndex >= PWM_CHANNELS_COUNT ||
        isnanf(startTemperature) || isnan(startPower) ||
        isnanf(stopTemperature) || isnan(stopPower) ||
        isnanf(maximumTemperature) || isnan(maximumPower) ||
        stopTemperature > startTemperature ||
        stopPower > startPower ||
        stopPower > maximumPower)
    {
      messageData.source->println("INVLD");
    }
    else
    {
      fanControlData[channelIndex].settings.spoolUpTime = spoolUpTime;
      fanControlData[channelIndex].settings.spoolDownTime = spoolDownTime;
      fanControlData[channelIndex].settings.startTemperature = startTemperature;
      fanControlData[channelIndex].settings.startPower = startPower;
      fanControlData[channelIndex].settings.minimalTemperature = stopTemperature;
      fanControlData[channelIndex].settings.minimalPower = stopPower;
      fanControlData[channelIndex].settings.maximalTemperature = maximumTemperature;
      fanControlData[channelIndex].settings.maximalPower = maximumPower;

      messageData.source->println("OK");
    }
  }
  else if (command == "SET_TEMP_CONTROL_CONST_SPEED")
  {
    String channelIndexString = getValue(messageData.message, ' ', 1);
    String powerString = getValue(messageData.message, ';', 2);

    auto channelIndex = static_cast<size_t>(channelIndexString.toInt());
    double power = powerString.toDouble();

    if (channelIndexString.length() == 0 || powerString.length() == 0 ||
      isnan(power) || power < 0.0 || power > 100.0 ||
      channelIndex >= PWM_CHANNELS_COUNT)
    {
      messageData.source->println("INVLD");
    }
    else
    {
      fanControlData[channelIndex].settings.spoolUpTime = 0;
      fanControlData[channelIndex].settings.spoolDownTime = 0;
      fanControlData[channelIndex].settings.startTemperature = MINIMAL_TEMPERATURE_C;
      fanControlData[channelIndex].settings.startPower = power;
      fanControlData[channelIndex].settings.minimalTemperature = MINIMAL_TEMPERATURE_C;
      fanControlData[channelIndex].settings.minimalPower = power;
      fanControlData[channelIndex].settings.maximalTemperature = MAXIMAL_TEMPERATURE_C;
      fanControlData[channelIndex].settings.maximalPower = power;

      messageData.source->println("OK");
    }
  }
  else if (command == "SET_TEMPERATURE_SOURCES")
  {
    // SET_TEMPERATURE_SOURCES 0 11
    // SET_TEMPERATURE_SOURCES 0 10
    // SET_TEMPERATURE_SOURCES 0 01
    // SET_TEMPERATURE_SOURCES 0 00
    // SET_TEMPERATURE_SOURCES 0 11000000000000000000000000000001
    // SET_TEMPERATURE_SOURCES 0 10000000000000000000000000000001
    // SET_TEMPERATURE_SOURCES 0 01000000000000000000000000000001
    // SET_TEMPERATURE_SOURCES 0 00000000000000000000000000000001
    // SET_TEMPERATURE_SOURCES 0 00000000000000000000000000000000

    String channelIndexString = getValue(messageData.message, ' ', 1);
    String temperatureSourcesString = getValue(messageData.message, ' ', 2);

    auto channelIndex = static_cast<size_t>(channelIndexString.toInt());

    if (channelIndexString.length() == 0 || temperatureSourcesString.length() == 0 ||
        channelIndex >= PWM_CHANNELS_COUNT)
    {
      messageData.source->println("INVLD");
    }
    else
    {
      size_t sensorsToSet = min(temperatureSourcesString.length(), static_cast<size_t>(MAXIMUM_TEMPERATURE_SENSORS + 1));
      for (size_t i = 0; i < sensorsToSet; i++)
      {
        if (temperatureSourcesString[i] == '1')
          fanControlData[channelIndex].settings.temperatureSourceSelect.set(i);
        else
          fanControlData[channelIndex].settings.temperatureSourceSelect.reset(i);
      }

      messageData.source->println("OK");
    }
  }
  else if (command == "ENABLE_CONTROL")
  {
    // ENABLE_CONTROL 0

    String channelIndexString = getValue(messageData.message, ' ', 1);
    auto channelIndex = static_cast<size_t>(channelIndexString.toInt());

    if (channelIndexString.length() == 0 || channelIndex >= PWM_CHANNELS_COUNT)
    {
      messageData.source->println("INVLD");
    }
    else
    {
      fanControlData[channelIndex].settings.enabled = true;
      messageData.source->println("OK");
    }
  }
  else if (command == "DISABLE_CONTROL")
  {
    // DISABLE_CONTROL 0

    String channelIndexString = getValue(messageData.message, ' ', 1);
    auto channelIndex = static_cast<size_t>(channelIndexString.toInt());

    if (channelIndexString.length() == 0 || channelIndex >= PWM_CHANNELS_COUNT)
    {
      messageData.source->println("INVLD");
    }
    else
    {
      fanControlData[channelIndex].settings.enabled = false;
      messageData.source->println("OK");
    }
  }
  else if (command == "SAVE_FAN_SETTINGS")
  {
    // SAVE_FAN_SETTINGS 0

    String channelIndexString = getValue(messageData.message, ' ', 1);
    auto channelIndex = static_cast<size_t>(channelIndexString.toInt());

    if (channelIndexString.length() == 0 || channelIndex >= PWM_CHANNELS_COUNT)
    {
      messageData.source->println("INVLD");
    }
    else
    {
      fanConfigs.putBytes(std::to_string(channelIndex).c_str(), &fanControlData[channelIndex].settings, sizeof(FanSettings));
      messageData.source->println("OK");
    }
  }
  else if (command == "SET_EXTERNAL_TEMPERATURE")
  {
    // SET_EXTERNAL_TEMPERATURE -100
    // SET_EXTERNAL_TEMPERATURE 25
    // SET_EXTERNAL_TEMPERATURE 30
    // SET_EXTERNAL_TEMPERATURE 35
    // SET_EXTERNAL_TEMPERATURE 50
    // SET_EXTERNAL_TEMPERATURE 60

    String externalTemperatureString = getValue(messageData.message, ' ', 1);
    float externalTemperatureTmp = externalTemperatureString.toFloat();

    if (externalTemperatureString.length() == 0 || isnanf(externalTemperatureTmp) ||
        externalTemperatureTmp > MAXIMAL_TEMPERATURE_C || externalTemperatureTmp < MINIMAL_TEMPERATURE_C)
    {
      messageData.source->println("INVLD");
    }
    else
    {
      externalTemperature = externalTemperatureTmp;
      messageData.source->println("OK");
    }
  }
  else if (command == "SET_WIFI_CREDENTIALS")
  {
    String ssidString = getValue(messageData.message, ' ', 1);
    String passwordString = getValue(messageData.message, ' ', 2);

    if (ssidString.length() == 0)
    {
      messageData.source->println("INVLD");
    }
    else
    {
      wifiSetCredentials(ssidString.c_str(), passwordString.c_str());
      messageData.source->println("OK");
    }
  }
  else if (command == "SET_HOSTNAME")
  {
    String hostnameString = getValue(messageData.message, ' ', 1);

    if (hostnameString.length() == 0)
    {
      messageData.source->println("INVLD");
    }
    else
    {
      connectionSettings.putString("hostname", hostnameString);

      MDNS.end();
      vTaskDelay(pdMS_TO_TICKS(5));

      if (MDNS.begin(hostnameString))
        messageData.source->println("OK");
      else
        messageData.source->println("ERROR");
    }
  }
  else if (command == "DEFAULT")
  {
    nvs_flash_erase();
    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(5));

    messageData.source->println("OK");
    messageData.source->flush();
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP.restart();
  }
  else if (command == "RESTART")
  {
    messageData.source->println("OK");
    messageData.source->flush();
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP.restart();
  }
  else if (command == "HELP")
  {
    messageData.source->println("Commands:");
    messageData.source->println("PING - Ping the device");
    messageData.source->println("TEMPERATURES - Print all temperatures from sensors");
    messageData.source->println("FAN_DATA [channel] - Print informations about fans and their settings");
    messageData.source->println("SET_FAN_SPEED <channel> <0-100/auto> - Set power of fan channel or switch it to auto management");
    messageData.source->println("TEST_START_FAN_SPEED <channel> - Test start speed of fan channel");
    messageData.source->flush();
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_task_wdt_reset();
    messageData.source->println("SET_TEMP_CONTROL_PARAMETERS <channel> <start_temp>;<start_power>;<stop_temp>;<min_power>;<max_temp>;<max_power>;<spoolup_time_ms>;<spooldown_time_ms> - Set temperature control parameters");
    messageData.source->printf("SET_TEMPERATURE_SOURCES <channel> <[0/1]*%d> - Select temperature sources, first %d are physical sensors and last is temperature set by command\n", MAXIMUM_TEMPERATURE_SENSORS + 1, MAXIMUM_TEMPERATURE_SENSORS);
    messageData.source->println("ENABLE_CONTROL <channel> - Enable automatic temperature control for channel");
    messageData.source->println("DISABLE_CONTROL <channel> - Disable automatic temperature control for channel");
    messageData.source->println("SAVE_FAN_SETTINGS <channel> - Save settings for channel");
    messageData.source->println("SET_EXTERNAL_TEMPERATURE <temp> - Set external temperature");
    messageData.source->flush();
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_task_wdt_reset();
    messageData.source->println("SET_WIFI_CREDENTIALS <ssid> [<pass>] - Set wifi credentials");
    messageData.source->println("SET_HOSTNAME <hostname> - Set device hostname WITHOUT extension, there will be .local extension appended");
    messageData.source->println("FAN_SPEED_SAMPLING_TIME - Check sampling time of fan RPM");
    messageData.source->println("DEFAULT - Clear all set data from EEPROM and restart");
    messageData.source->println("RESTART - Restart the device");
    messageData.source->printf("Temperatures can be set from %.1f to %.1f\n", MINIMAL_TEMPERATURE_C, MAXIMAL_TEMPERATURE_C);
    messageData.source->flush();
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_task_wdt_reset();
    messageData.source->println("Power can be set from 0 to 100 and its in %");
    messageData.source->println("Temperature sources in FAN_DATA are reversed!");
    messageData.source->printf("Channels are 0 - %d\n", PWM_CHANNELS_COUNT - 1);
  }
  else
  {
    messageData.source->println("UNKN");
  }
}

void enqueMessage(Print* source, const String& message)
{
  messageQueue.push({ message, source });
}

[[noreturn]] void processMessageTask(void*)
{
  if (esp_task_wdt_add(nullptr) != ESP_OK)
  {
    LOG_ERROR("Failed to add message processing task to WDT");
  }

  while (true)
  {
    if (!messageQueue.empty())
    {
      processMessage(messageQueue.front());
      esp_task_wdt_reset();
      messageQueue.pop();
    }

    vTaskDelay(pdMS_TO_TICKS(5));
    esp_task_wdt_reset();
  }
}

void messageHandlerInit()
{
  xTaskCreateUniversal(processMessageTask, "msg_proc", 4096, nullptr, 3, nullptr, 1);
}
