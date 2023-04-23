#include <Arduino.h>
#include <ArduinoOTA.h>

#include "settings.h"

void ota_handle( void * parameter ) 
{
  while (true) 
  {
    ArduinoOTA.handle();
    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}

TaskHandle_t otaTaskHandle = NULL;

void OTA_setup()
{
#ifdef DEBUG
  ArduinoOTA.onStart([]() 
  {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else
      type = "filesystem";

    Serial.println("[OTA] Start updating " + type);
  });
  
  ArduinoOTA.onEnd([]() 
  {
    Serial.println("\nEnd");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) 
  {
    Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) 
  {
    Serial.printf("[OTA] Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("\nAuth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("\nBegin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("\nConnect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("\nReceive Failed");
    else if (error == OTA_END_ERROR) Serial.println("\nEnd Failed");
  });
#endif

  ArduinoOTA.begin();

  xTaskCreateUniversal(
    ota_handle,
    "OTA_HANDLE",
    8192,
    NULL,
    1,
    &otaTaskHandle,
    1);

#ifdef DEBUG
  Serial.println("[OTA] Initialized");
#endif
}

void OTA_end()
{
  if (otaTaskHandle != NULL)
  {
    vTaskDelete(otaTaskHandle);
    otaTaskHandle = NULL;
  }
  ArduinoOTA.end();
}
