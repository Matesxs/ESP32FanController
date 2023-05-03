#include "message_processing.h"

#include "settings.h"

#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <sstream>
#include <nvs_flash.h>

#ifdef WIFI_CONNECTION
#include "connection/wifiHandling.h"

#ifdef ENABLE_OTA
#include "mic/OTA.h"
#endif
#endif

#ifdef LED_STRIPS_ENABLE
#include "led_strips/effects.h"
#endif

#include "mic/utils.h"
#include "mic/data_share.h"
#include "temperature_reader.h"

DynamicJsonDocument generateData()
{
  DynamicJsonDocument doc(16384);
  doc["temp_sens"][0]["id"] = 1;
  doc["temp_sens"][0]["val"] = externalTemperature;
  for (uint8_t i = 0; i < temp_reader_sensor_count(); i++)
  {
    doc["temp_sens"][i + 1]["id"] = pow(2, i + 1);
    doc["temp_sens"][i + 1]["val"] = temperatures[i];
  }

  for (size_t i = 0; i < FAN_COUNT; i++)
  {
    doc["fans"][i]["id"] = i;
    doc["fans"][i]["current_power"] = fanData[i].currentPower;
    doc["fans"][i]["rmp"] = fanData[i].rpm;
    doc["fans"][i]["manual"] = fanData[i].manual;
    doc["fans"][i]["curve"]["temp_source"] = fanData[i].curve.tempSource;

    double curve_temps[NUMBER_OF_CURVE_POINTS];
    double curve_setpoints[NUMBER_OF_CURVE_POINTS];
    fanData[i].curve.getCurve(curve_temps, curve_setpoints);

    copyArray(curve_temps, doc["fans"][i]["curve"]["points"]["temperatures"]);
    copyArray(curve_setpoints, doc["fans"][i]["curve"]["points"]["setpoints"]);
  }

  if (ledStripChannel1 != NULL)
  {
    doc["led_strips"][0]["length"] = ledStripChannel1->getLength();
    doc["led_strips"][0]["effect"] = ledStripChannel1->getCurrentEffect();
  }

  if (ledStripChannel2 != NULL)
  {
    doc["led_strips"][1]["length"] = ledStripChannel2->getLength();
    doc["led_strips"][1]["effect"] = ledStripChannel2->getCurrentEffect();
  }

  return doc;
}

String processComMessage(String data)
{
  String returnData;

  String command = getSubstring(data, ' ', 0);

#ifdef WIFI_CONNECTION
  if (command == "SET_WIFI_CREDENTIALS")
  {
    // SET_WIFI_CREDENTIALS <SSID> <password>

    String ssid = getSubstring(data, ' ', 1);
    String password = getSubstring(data, ' ', 2);

    if (ssid == "" || password == "")
    {
      returnData = format_string("INVLD %s\r\n", data.c_str());
    }
    else
    {
      wifi_setCredentials(ssid.c_str(), password.c_str());
      mainSettings.putString("wifi_ssid", ssid);
      mainSettings.putString("wifi_pass", password);

#ifdef CUSTOM_DNS
      MDNS.end();
      MDNS.begin(mainSettings.isKey("dns_name") ? mainSettings.getString("dns_name", DEFAULT_DNS_NAME).c_str() : DEFAULT_DNS_NAME);
#endif

      returnData = format_string("OK SET_WIFI_CREDENTIALS %s %s\r\n", ssid.c_str(), password.c_str());
    }
  }
  else if (command == "WIFI_FORGET")
  {
    wifi_end();
    mainSettings.remove("wifi_ssid");
    mainSettings.remove("wifi_pass");
    returnData = format_string("OK %s\r\n", data.c_str());
  }
#ifdef CUSTOM_DNS
  else if (command == "SET_HOST_NAME")
  {
    // SET_HOST_NAME esp32_pwm_controler

    String hostname = getSubstring(data, ' ', 1);

    if (hostname == "")
    {
      returnData = format_string("INVLD %s\r\n", data.c_str());
    }
    else
    {
      hostname.replace(" ", "_");
      MDNS.end();
      MDNS.begin(hostname.c_str());
      mainSettings.putString("dns_name", hostname);
      returnData = format_string("OK SET_HOST_NAME %s\r\n", hostname.c_str());
    }
  }
#endif
  else
#endif
      if (command == "TEMP_COUNT")
  {
    returnData = format_string("OK TEMP_COUNT %d\r\n", temp_reader_sensor_count());
  }
  else if (command == "GET_TEMPS")
  {
    String final_string = "OK GET_TEMPS";
    for (uint8_t i = 0; i < temp_reader_sensor_count(); i++)
      final_string += String(" " + String(temperatures[i], 2));
    returnData = format_string("%s\r\n", final_string.c_str());
  }
  else if (command == "SET_FAN")
  {
    // SET_FAN 0 80
    // SET_FAN 0 10.5
    // SET_FAN 0 AUTO

    String fanIndexString = getSubstring(data, ' ', 1);
    String fanSpeedString = getSubstring(data, ' ', 2);
    int fanIndex = fanIndexString.toInt();
    double fanSpeed = fanSpeedString.toDouble();

    if (fanIndexString == "" || fanSpeedString == "" ||
        isnan(fanIndex) || fanIndex < 0 || fanIndex >= FAN_COUNT ||
        (fanSpeedString != "AUTO" && (isnan(fanSpeed) || fanSpeed < 0.0 || fanSpeed > 100.0)))
    {
      returnData = format_string("INVLD %s\r\n", data.c_str());
    }
    else
    {
      if (fanSpeedString == "AUTO")
      {
        fanData[fanIndex].manual = false;
        fanData[fanIndex].dirty = true;
      }
      else
      {
        fanData[fanIndex].manual = true;
        fanData[fanIndex].lerper.set(fanSpeed, FAN_SPEED_RAMP_TIME_MS);
      }

      returnData = format_string("OK %s\r\n", data.c_str());
    }
  }
  else if (command == "SET_CURVE")
  {
    // SET_CURVE 0 40@20;60@50;80@100
    // SET_CURVE 0 40@100
    // SET_CURVE 0 40@0
    // SET_CURVE 1 30@35;60@50;90@100
    // SET_CURVE 1 0@0

    String fanIndexString = getSubstring(data, ' ', 1);
    String curveString = getSubstring(data, ' ', 2);
    int fanIndex = fanIndexString.toInt();

    if (fanIndexString == "" || curveString == "" ||
        isnan(fanIndex) || fanIndex < 0 || fanIndex >= FAN_COUNT)
    {
      returnData = format_string("INVLD %s\r\n", data.c_str());
    }
    else
    {
      double lastTemp = 0;
      double lastSetpoint = 0;
      for (size_t i = 0; i < NUMBER_OF_CURVE_POINTS; i++)
      {
        String pointString = getSubstring(curveString, ';', i);
        String tempString = getSubstring(pointString, '@', 0);
        String setpointString = getSubstring(pointString, '@', 1);
        double temp = tempString.toDouble();
        double setpoint = setpointString.toDouble();

        if (pointString == "" || tempString == "" || setpointString == "" ||
            isnan(temp) ||
            isnan(setpoint) || setpoint < 0 || setpoint > 100)
        {
          fanData[fanIndex].curve.points[i].set(lastTemp, lastSetpoint);
        }
        else
        {
          fanData[fanIndex].curve.points[i].set(temp, setpoint);
          lastTemp = temp;
          lastSetpoint = setpoint;
        }
      }

      fanData[fanIndex].curve.sort();

      curvesStore.putBytes(String(fanIndex).c_str(), static_cast<void *>(&fanData[fanIndex].curve), sizeof(TemperatureCurve));

      fanData[fanIndex].dirty = true;
      returnData = format_string("OK %s\r\n", data.c_str());
    }
  }
  else if (command == "SET_SOURCE")
  {
    // SET_SOURCE 0 1
    // SET_SOURCE 1 6
    // SET_SOURCE 0 6

    String fanIndexString = getSubstring(data, ' ', 1);
    String sourceString = getSubstring(data, ' ', 2);
    int fanIndex = fanIndexString.toInt();
    uint64_t source;
    std::istringstream iss(sourceString.c_str());
    iss >> source;

    if (fanIndexString == "" || sourceString == "" ||
        isnan(fanIndex) || fanIndex < 0 || fanIndex >= FAN_COUNT ||
        isnan(source))
    {
      returnData = format_string("INVLD %s\r\n", data.c_str());
    }
    else
    {
      fanData[fanIndex].curve.tempSource = source;
      curvesStore.putBytes(String(fanIndex).c_str(), static_cast<void *>(&fanData[fanIndex].curve), sizeof(TemperatureCurve));

      fanData[fanIndex].dirty = true;
      returnData = format_string("OK %s\r\n", data.c_str());
    }
  }
  else if (command == "SET_EXT_TEMP")
  {
    // SET_EXT_TEMP 35
    // SET_EXT_TEMP 42
    // SET_EXT_TEMP 59
    // SET_EXT_TEMP 61
    // SET_EXT_TEMP 65
    // SET_EXT_TEMP 67
    // SET_EXT_TEMP 90

    String extTempString = getSubstring(data, ' ', 1);
    float extTemp = extTempString.toFloat();

    if (extTempString == "" ||
        isnan(extTemp))
    {
      returnData = format_string("INVLD %s\r\n", data.c_str());
    }
    else
    {
      externalTemperature = extTemp;
      returnData = format_string("OK SET_EXT_TEMP %.2f\r\n", extTemp);
    }
  }
#ifdef LED_STRIPS_ENABLE
  else if (command == "SET_LED_COUNT")
  {
    // SET_LED_COUNT 1 10
    // SET_LED_COUNT 2 15

    String ledChannelString = getSubstring(data, ' ', 1);
    String ledCountString = getSubstring(data, ' ', 2);
    int ledChannel = ledChannelString.toInt();
    int ledCount = ledCountString.toInt();
    uint16_t ledCountConverted = static_cast<uint16_t>(ledCount);

    if (ledChannelString == "" || ledCountString == "" ||
        isnan(ledChannel) || (ledChannel != 1 && ledChannel != 2) ||
        isnan(ledCount) || ledCount < 0 || ledCount != ledCountConverted)
    {
      returnData = format_string("INVLD %s\r\n", data.c_str());
    }
    else
    {
      switch (ledChannel)
      {
      case 1:
        ledStripChannel1 = new StripHandler(ledCountConverted, LED_STRIP1_DATA_PIN);
        mainSettings.putUShort("led_ch1_c", ledCountConverted);
        break;

      case 2:
        ledStripChannel2 = new StripHandler(ledCountConverted, LED_STRIP2_DATA_PIN);
        mainSettings.putUShort("led_ch2_c", ledCountConverted);
        break;

      default:
        break;
      }

      returnData = format_string("OK %s\r\n", data.c_str());
    }
  }
  else if (command == "SET_LED")
  {
    // SET_LED 1 off
    // SET_LED 2 off
    // SET_LED 1 solid@15;30;180
    // SET_LED 2 wave@200;5;110
    // SET_LED 1 tsolid@1;20;50
    // SET_LED 2 twave@1;10;80
    // SET_LED 1 rainbow@100
    // SET_LED 2 kitt@200;5;10
    // SET_LED 1 breath@120;5;180
    // SET_LED 2 breath@18;210;100&2000

    String ledChannelString = getSubstring(data, ' ', 1);
    String settingString = getSubstring(data, ' ', 2);
    int ledChannel = ledChannelString.toInt();
    if (ledChannelString == "" || settingString == "" ||
        isnan(ledChannel) || (ledChannel != 1 && ledChannel != 2))
    {
      returnData = format_string("INVLD %s\r\n", data.c_str());
    }
    else if ((ledChannel == 1 && ledStripChannel1 == NULL) ||
             (ledChannel == 2 && ledStripChannel2 == NULL))
    {
      returnData = format_string("UNSET %s\r\n", data.c_str());
    }
    else
    {
      bool result = false;
      switch (ledChannel)
      {
      case 1:
        if (result = ledStripChannel1->setTarget(settingString))
          mainSettings.putString("led_ch1_m", settingString);
        break;

      case 2:
        if (result = ledStripChannel2->setTarget(settingString))
          mainSettings.putString("led_ch2_m", settingString);
        break;

      default:
        break;
      }

      if (result)
        returnData = format_string("OK %s\r\n", data.c_str());
      else
        returnData = format_string("INVLD %s\r\n", data.c_str());
    }
  }
#endif
  else if (command == "GET_DATA")
  {
    auto doc = generateData();

    String finalData;
    serializeJson(doc, finalData);

    returnData = format_string("OK GET_DATA %s\r\n", finalData.c_str());
  }
  else if (command == "GET_DATA_PRETTY")
  {
    auto doc = generateData();

    String finalData;
    serializeJsonPretty(doc, finalData);

    returnData = format_string("OK GET_DATA_PRETTY\r\n%s\r\n", finalData.c_str());
  }
  else if (command == "RESTART")
  {
    returnData = format_string("OK RESTART\r\n");
#ifdef WIFI_CONNECTION
#ifdef ENABLE_OTA
    OTA_end();
#endif
    wifi_end();
#endif
    ESP.restart();
  }
  else if (command == "FACTORY")
  {
    mainSettings.clear();
    curvesStore.clear();
    returnData = format_string("OK FACTORY\r\n");
  }
#ifdef ENABLE_DEBUG
  else if (command == "HARD_CLEAR")
  {
    nvs_flash_erase(); // erase the NVS partition and...
    nvs_flash_init();  // initialize the NVS partition.
    returnData = format_string("OK HARD_CLEAR\r\n");
  }
#endif
  else
  {
    returnData = format_string("UNKCOM %s\r\n", command.c_str());
  }

  return returnData;
}