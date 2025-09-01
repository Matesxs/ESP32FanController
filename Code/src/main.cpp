#include <Arduino.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <esp_task_wdt.h>

#include <WebSerialLite.h>
#include <custom_debug.h>
#include <pwmControl.h>

#include "pins.h"
#include "settings.h"
#include "wifi_handler.h"

extern void tachoSensingInit();
extern void temperatureReaderInit();
extern void temeperatureControllerInit();
extern void pollUart();
extern void webserverInit();
extern void pollWebserver();
extern void otaInit();
extern void messageHandlerInit();

extern FanControlData fanControlData[PWM_CHANNELS_COUNT];
extern Preferences fanConfigs;
extern Preferences connectionSettings;

static uint32_t lastPollUart = 0;
static uint32_t lastPollWebserver = 0;
[[noreturn]] void interfaceHandlingTask(void*)
{
  if (esp_task_wdt_add(nullptr) != ESP_OK)
  {
    LOG_ERROR("Failed to add interface handling task to WDT");
  }

  while (true)
  {
    const uint32_t now = millis();
    if (now - lastPollUart > UART_POLL_INTERVAL_MS)
    {
      pollUart();
      esp_task_wdt_reset();
      lastPollUart = now;
    }

    if (now - lastPollWebserver > WEBSERVER_UPDATE_INTERVAL_MS)
    {
      pollWebserver();
      esp_task_wdt_reset();
      lastPollWebserver = now;
    }

    vTaskDelay(pdMS_TO_TICKS(5));
    esp_task_wdt_reset();
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(500);
  vTaskDelay(pdMS_TO_TICKS(1000));

  LOG_INFO("Starting initialization...");

  LOG_INFO("Setup PWM timer");
  for (const auto& timer : PWM_TIMERS)
    PWMControl::setupTimer(timer, FAN_PWM_RESOLUTION, FAN_PWM_FREQUENCY_HZ);

  LOG_INFO("Init IO");
  for (const auto& [tachoPin, pwmPin, pwmTimer, pwmChannel, invert] : FAN_PINS_HOLDER)
  {
    // Init fan pins
    pinMode(tachoPin, INPUT);
    pinMode(pwmPin, OUTPUT);

    // Init PWM control
    PWMControl::attachPin(pwmTimer, pwmChannel, pwmPin, invert);
  }

  for (const auto& pwmChannel : PWM_CHANNELS)
    PWMControl::write(pwmChannel, FAN_PWM_MAX_VALUE); // Full speed before speed control

  LOG_INFO("Init WiFi");
  wifiInit();
  vTaskDelay(pdMS_TO_TICKS(10));

  LOG_INFO("Loading configs from EEPROM");
  if (fanConfigs.begin("fan_cfg", false))
  {
    for (size_t i = 0; i < PWM_CHANNELS_COUNT; i++)
    {
      std::string key = std::to_string(i);
      if (fanConfigs.isKey(key.c_str()) && fanConfigs.getBytesLength(key.c_str()) == sizeof(FanSettings))
        fanConfigs.getBytes(key.c_str(), &fanControlData[i].settings, sizeof(FanSettings));
    }
  }
  else
  {
    LOG_ERROR("Failed to open fan configs partition");
  }

#ifdef ENABLE_WIFI
  if (connectionSettings.begin("conn", false))
  {
    LOG_INFO("Starting WiFi");
    wifiEnable();

    if (connectionSettings.isKey("hostname"))
    {
      char hostnameBuffer[65] = {};
      connectionSettings.getString("hostname", hostnameBuffer, std::size(hostnameBuffer) - 1);
      if (!MDNS.begin(hostnameBuffer))
      {
        LOG_ERROR("Failed to set custom hostname, trying default one...");

        if (!MDNS.begin(DEFAULT_HOSTNAME))
        {
          LOG_ERROR("Failed to set hostname");
        }
      }
    }
    else
    {
      if (!MDNS.begin(DEFAULT_HOSTNAME))
      {
        LOG_ERROR("Failed to set hostname");
      }
    }
  }
  else
  {
    LOG_ERROR("Failed to open connection settings partition");
  }
#endif

  // Enable watchdog even on main core
  enableCore1WDT();

  LOG_INFO("Initialization finished");
  vTaskDelay(pdMS_TO_TICKS(100));

  LOG_INFO("Starting handlers");
  temperatureReaderInit();
  tachoSensingInit();
  temeperatureControllerInit();
  messageHandlerInit();

#ifdef ENABLE_WIFI
  webserverInit();
#ifdef ENABLE_OTA
  otaInit();
#endif
#endif

  xTaskCreateUniversal(interfaceHandlingTask, "interface", 4096, nullptr, 4, nullptr, 1);
}

void loop()
{
  vTaskDelete(nullptr);
}
