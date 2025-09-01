//
// Created by Martin on 5. 7. 2025.
//

#include <Arduino.h>
#include <esp_task_wdt.h>

#include <custom_debug.h>
#include <moving_average.h>

#include "datastores.h"
#include "pins.h"
#include "settings.h"
#include "helpers.h"

static MovingAverage<uint32_t, FAN_SPEED_AVERAGE_SAMPLES> averageFanRPM[FAN_COUNT];
static uint64_t lastYield = 0;

extern uint32_t currentFanSamplingTime;
extern uint32_t avgFanRPM[FAN_COUNT];

uint32_t getSpeed(const uint8_t sensorPin, const uint8_t measure_samples, const uint32_t pulses_per_rotation, const uint32_t number_of_poles)
{
  uint32_t returnValue = 0;
  const uint32_t timeCheck = pulseIn(sensorPin, LOW, 200000); // Timeout in microseconds
  yieldIfNecessary(lastYield);
  esp_task_wdt_reset();

  if (timeCheck != 0)
  {
    uint32_t time = 0;

    for (uint8_t i = 0; i < measure_samples; i++)
    {
      const uint32_t tmp = pulseIn(sensorPin, LOW, 200000); // Timeout in microseconds
      yieldIfNecessary(lastYield);
      esp_task_wdt_reset();

      if (tmp == 0) return returnValue;
      time += tmp;
    }

    if (time > 0)
    {
      const uint32_t averageLowPulseLength = time / measure_samples;
      const uint32_t averageTimeFullRotation = averageLowPulseLength * pulses_per_rotation * 2;
      const float turnsPerMicrosecond = 1.0f / static_cast<float>(averageTimeFullRotation);
      const float turnsPerSecond = turnsPerMicrosecond * 1000.0f * 1000.0f;
      const auto rpm = static_cast<uint32_t>(turnsPerSecond * 60.0f);
      returnValue = rpm / number_of_poles;
    }
  }

  return returnValue;
}

[[noreturn]] void pollFanRPMTask(void *)
{
  if (esp_task_wdt_add(nullptr) != ESP_OK)
  {
    LOG_ERROR("Failed to add fan rpm poll task to WDT");
  }

  while (true)
  {
    const uint32_t startSampling = millis();
    for (size_t i = 0; i < FAN_COUNT; i++)
    {
      avgFanRPM[i] = averageFanRPM[i](getSpeed(FAN_PINS_HOLDER[i].tachoPin, SAMPLES_PER_RPM_READING, FAN_PULSES_PER_ROTATION, FAN_NUMBER_OF_POLES));
      yieldIfNecessary(lastYield);
      esp_task_wdt_reset();

      // LOG_DEBUG("Fan %d - raw rpm: %d, avg rpm: %d", i, rawFanRPM[i], avgFanRPM[i]);
    }

    vTaskDelay(pdMS_TO_TICKS(FAN_SPEED_SAMPLING_INTERVAL_MS));
    esp_task_wdt_reset();

    currentFanSamplingTime = millis() - startSampling;
  }
}

void tachoSensingInit()
{
  // Wait for spoolup for debug detection
  vTaskDelay(pdMS_TO_TICKS(FAN_POWERON_BUMP_DURATION_MS));

  size_t runningFansCount = 0;
  for (size_t i = 0; i < FAN_COUNT; i++)
  {
    avgFanRPM[i] = getSpeed(FAN_PINS_HOLDER[i].tachoPin, SAMPLES_PER_RPM_READING, FAN_PULSES_PER_ROTATION, FAN_NUMBER_OF_POLES);
    averageFanRPM[i].Fill(avgFanRPM[i]);
    if (avgFanRPM[i] > 0) runningFansCount++;

    yieldIfNecessary(lastYield);
  }

  if (runningFansCount > 0)
  {
    LOG_DEBUG("Detected %d running fans", runningFansCount);
  }
  else
  {
    LOG_ERROR("No running fans detected");
  }

  xTaskCreateUniversal(pollFanRPMTask, "tacho_sen", 1024, nullptr, 2, nullptr, 1);
}
