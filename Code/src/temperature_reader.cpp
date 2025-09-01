//
// Created by Martin on 5. 7. 2025.
//

#include <DallasTemperature.h>
#include <esp_task_wdt.h>

#include <custom_debug.h>
#include <moving_average.h>

#include "pins.h"
#include "settings.h"
#include "helpers.h"
#include "datastores.h"

static OneWire temperatureSensorsBus(ONE_WIRE_BUS);
static DallasTemperature temperatureSensors(&temperatureSensorsBus); // 1k odpor

static float rawTemperatures[MAXIMUM_TEMPERATURE_SENSORS];
static MovingAverage<float, TEMPERATURE_AVERAGE_SAMPLES> temperatureMovingAverage[MAXIMUM_TEMPERATURE_SENSORS];

static uint64_t lastYield = 0;

extern TemperatureData temperatureData[MAXIMUM_TEMPERATURE_SENSORS];
extern uint8_t detectedTemperatureSensors;

[[noreturn]] void pollTemperatureTask(void *)
{
  if (esp_task_wdt_add(nullptr) != ESP_OK)
  {
    LOG_ERROR("Failed to add temperature reader task to WDT");
  }

  while (true)
  {
    temperatureSensors.requestTemperatures();
    esp_task_wdt_reset();

    for (uint8_t i = 0; i < detectedTemperatureSensors; i++)
    {
      rawTemperatures[i] = temperatureSensors.getTempCByIndex(i);
      temperatureData[i].temperature = temperatureMovingAverage[i](rawTemperatures[i]);

      if (rawTemperatures[i] < MINIMAL_TEMPERATURE_C || rawTemperatures[i] > MAXIMAL_TEMPERATURE_C) temperatureData[i].valid = false;
      else temperatureData[i].valid = true;

      //LOG_DEBUG("Temperature %d (working: %d) - cur: %.1f°C, avg: %.1f°C", i, temperatureData[i].valid, rawTemperatures[i], temperatureData[i].temperature);
      yieldIfNecessary(lastYield);
      esp_task_wdt_reset();
    }

    vTaskDelay(pdMS_TO_TICKS(TEMPERATURE_SAMPLING_INTERVAL_MS));
    esp_task_wdt_reset();
  }
}

void temperatureReaderInit()
{
  temperatureSensors.begin();
  detectedTemperatureSensors = temperatureSensors.getDeviceCount();

  if (detectedTemperatureSensors > MAXIMUM_TEMPERATURE_SENSORS)
  {
    detectedTemperatureSensors = MAXIMUM_TEMPERATURE_SENSORS;
    LOG_INFO("Detected more temperature sensors than allowed, only %d sensors will be used", MAXIMUM_TEMPERATURE_SENSORS);
  }

  if (detectedTemperatureSensors > 0)
  {
    LOG_DEBUG("Detected %d temperature sensors", detectedTemperatureSensors);

    temperatureSensors.requestTemperatures();
    vTaskDelay(pdMS_TO_TICKS(1));

    for (uint8_t i = 0; i < detectedTemperatureSensors; i++)
    {
      temperatureData[i].valid = true;

      rawTemperatures[i] = temperatureSensors.getTempCByIndex(i);
      temperatureMovingAverage[i].Fill(rawTemperatures[i]);
      temperatureData[i].temperature = rawTemperatures[i];
      yieldIfNecessary(lastYield);
    }
  }
  else
  {
    LOG_ERROR("No temperature sensors found");
  }

  xTaskCreateUniversal(pollTemperatureTask, "temp_sen", 1024, nullptr, 2, nullptr, 1);
}


