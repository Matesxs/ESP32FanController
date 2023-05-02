#include "settings.h"

#include <Arduino.h>

#ifdef WIFI_CONNECTION
#ifdef CUSTOM_DNS
#include <ESPmDNS.h>
#endif

#include "connection/wifiHandling.h"

#ifdef ENABLE_OTA
#include "mic/OTA.h"
#endif

#ifdef WIFI_CONTROL
#include "communication/tcp_server.h"
#endif
#endif

#ifdef COM_SERIAL
#ifdef USE_ARGUS_INTERFACE
#include "communication/argus_com.h"
#endif
#endif

#ifdef LED_STRIPS_ENABLE
#include "led_strips/led_strips_handling_routine.h"
#include "led_strips/effects.h"
#endif

#include "mic/debug.h"
#include "pins.h"
#include "mic/data_share.h"
#include "tacho.h"
#include "temperature_reader.h"
#include "fan_control_loop.h"
#include "communication/message_processing.h"
#include "communication/communication_handler.h"

void setup()
{
#ifdef COM_SERIAL
  if (COM_SERIAL != Serial)
  {
    Serial.begin(115200);
    while (!Serial);
  }

#ifndef USE_ARGUS_INTERFACE
  COM_SERIAL.begin(115200);
  while (!COM_SERIAL);
#else
  COM_SERIAL.begin(57600);
  while (!COM_SERIAL);
#endif
#endif

  mainSettings.begin("main_settings", false);
  curvesStore.begin("curves", false);
  delay(50);

#ifdef WIFI_CONNECTION
  wifi_setup();
  wifi_setCredentials(mainSettings.isKey("wifi_ssid") ? mainSettings.getString("wifi_ssid", "").c_str() : "", mainSettings.isKey("wifi_pass") ? mainSettings.getString("wifi_pass", "").c_str() : "");
  wifi_enable();
#ifdef ENABLE_OTA
  OTA_setup();
#endif

#ifdef CUSTOM_DNS
  if (!MDNS.begin(mainSettings.isKey("dns_name") ? mainSettings.getString("dns_name", DEFAULT_DNS_NAME).c_str() : DEFAULT_DNS_NAME)) 
  {
    DEBUG("[MDNS] Error setting up MDNS responder!\r\n");
  }
#endif
#endif

#ifdef LED_STRIPS_ENABLE
  if (mainSettings.isKey("led_ch1_c"))
  {
    uint16_t numberOfLeds = mainSettings.getUShort("led_ch1_c", 10);

    DEBUG("[Main] Found led strip channel settings for channel 1 (%d)\r\n", numberOfLeds);

    ledStripChannel1 = new StripHandler(numberOfLeds, LED_STRIP1_DATA_PIN);

    if (mainSettings.isKey("led_ch1_m"))
    {
      DEBUG("[Main] Found led strip effect settings for channel 1\r\n");

      ledStripChannel1->setTarget(mainSettings.getString("led_ch1_m"));
    }
  }

  if (mainSettings.isKey("led_ch2_c"))
  {
    uint16_t numberOfLeds = mainSettings.getUShort("led_ch2_c", 10);

    DEBUG("[Main] Found led strip channel settings for channel 2 (%d)\r\n", numberOfLeds);

    ledStripChannel2 = new StripHandler(numberOfLeds, LED_STRIP2_DATA_PIN);

    if (mainSettings.isKey("led_ch2_m"))
    {
      DEBUG("[Main] Found led strip effect settings for channel 2\r\n");

      ledStripChannel2->setTarget(mainSettings.getString("led_ch2_m"));
    }
  }
#endif

  for (size_t i = 0; i < FAN_COUNT; i++)
  {
    auto pinSettings = FAN_PINS_HOLDER[i];
    pinMode(pinSettings.tachoPin, INPUT);
    pinMode(pinSettings.pwmPin, OUTPUT);
    ledcSetup(pinSettings.pwmChannel, FAN_PWM_FREQ, FAN_PWM_RES);
    ledcAttachPin(pinSettings.pwmPin, pinSettings.pwmChannel);
    ledcWrite(pinSettings.pwmChannel, 0);
    
    fanData[i].lerper.setReference(&fanData[i].currentPower);
    
    String key(i);

    if (curvesStore.isKey(key.c_str()))
    {
      DEBUG("[Main] Found curve for fan %d\r\n", i);

      if (curvesStore.getBytes(key.c_str(), static_cast<void*>(&fanData[i].curve), sizeof(TemperatureCurve)) != sizeof(TemperatureCurve))
        fanData[i].curve.setConstant(DEFAULT_PWM_PERCENT);
    }
    else
      fanData[i].curve.setConstant(DEFAULT_PWM_PERCENT);
      
    fanData[i].curve.sort();
    fanData[i].dirty = true;
  }

  tacho_begin();
  temp_read_begin();
  begin_fan_control();
#ifdef WIFI_CONNECTION
#ifdef WIFI_CONTROL
  begin_tcp_server();
#endif
#endif

#ifdef LED_STRIPS_ENABLE
  begin_led_strips();
#endif

#ifdef COM_SERIAL
#ifdef USE_ARGUS_INTERFACE
  begin_argus_com();
#endif
#endif

  begin_communication();
}

void loop()
{
  vTaskDelete(NULL);
}
