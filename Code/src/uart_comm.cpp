//
// Created by Martin on 6. 7. 2025.
//

#include <Arduino.h>
#include <esp_task_wdt.h>

extern void enqueMessage(Print* source, const String& message);

void pollUart()
{
  if (Serial.available())
  {
    String data = Serial.readStringUntil('\n');
    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(1));
    data.trim();

    enqueMessage(&Serial, data);
  }
}
