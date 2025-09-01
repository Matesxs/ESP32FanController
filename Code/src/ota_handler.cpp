//
// Created by Martin on 8. 7. 2025.
//

#include <Arduino.h>
#include <ArduinoOTA.h>

// This task is without WTD because it will hang in the function for a while while handling the OTA
[[noreturn]] void otaHandleTask(void *)
{
  while (true)
  {
    ArduinoOTA.handle();
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void otaInit()
{
  ArduinoOTA.setMdnsEnabled(false);
  ArduinoOTA.begin();

  xTaskCreateUniversal(
    otaHandleTask,
    "ota",
    8192,
    nullptr,
    5,
    nullptr,
    1);
}

