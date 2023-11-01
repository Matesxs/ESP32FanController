#include "tacho.h"

#include <Arduino.h>

#include "pins.h"
#include "mic/data_share.h"
#include "settings.h"

uint16_t getSpeed(int sensorPin, uint8_t measure_samples, uint32_t pulses_per_rotation, uint32_t number_of_poles)
{
  uint16_t returnValue = 0;
  uint32_t timeCheck = pulseIn(sensorPin, LOW);

  if (timeCheck != 0)
  {
    uint32_t time = 0;

    for (uint8_t i = 0; i < measure_samples; i++)
      time += pulseIn(sensorPin, LOW);

    if (time > 0)
    {
      uint32_t averageLowPulseLength = time / measure_samples;
      uint32_t averageTimeFullRotation = averageLowPulseLength * pulses_per_rotation * 2;
      float turnsPerMicrosecond = 1.0 / averageTimeFullRotation;
      float turnsPerSecond = turnsPerMicrosecond * 1000.0 * 1000.0;
      int16_t rpm = turnsPerSecond * 60;
      returnValue = rpm / number_of_poles;
    }
  }

  return returnValue;
}

void updateRPMTask(void *)
{
  while (true)
  {
    for (size_t i = 0; i < FAN_COUNT; i++)
    {
      fanData[i].rpm = getSpeed(FAN_PINS_HOLDER[i].tachoPin, SAMPLES_PER_RPM_READING, FAN_PULSES_PER_ROTATION, FAN_NUMBER_OF_POLES);
      vTaskDelay(pdMS_TO_TICKS(5));
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void tacho_begin()
{
  xTaskCreateUniversal(
    updateRPMTask,
    "fan_rmp",
    4096,
    NULL,
    1,
    NULL,
    0
  );
}
