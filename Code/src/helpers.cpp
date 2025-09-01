//
// Created by Martin on 6. 7. 2025.
//

#include <Arduino.h>

#include "helpers.h"

void yieldIfNecessary(uint64_t& lastYield, const uint64_t timeout)
{
  if(const uint64_t now = millis(); now - lastYield > timeout)
  {
    lastYield = now;
    vTaskDelay(pdMS_TO_TICKS(1)); //delay 1 RTOS tick
  }
}

void yieldIfNecessary()
{
  static uint64_t lastYield = 0;
  yieldIfNecessary(lastYield);
}
