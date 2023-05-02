#include <Arduino.h>
#include <ArduinoOTA.h>

#include "mic/debug.h"

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
  ArduinoOTA.begin();

  xTaskCreateUniversal(
    ota_handle,
    "OTA_HANDLE",
    8192,
    NULL,
    1,
    &otaTaskHandle,
    1);

  DEBUG("[OTA] Initialized\r\n");
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
