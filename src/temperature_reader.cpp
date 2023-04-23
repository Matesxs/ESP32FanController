#include "temperature_reader.h"

#include <Wire.h>
#include <DallasTemperature.h>

#include "settings.h"
#include "pins.h"

OneWire temperatureSensorsBus(ONE_WIRE_BUS);
DallasTemperature temperatureSensors(&temperatureSensorsBus); // 1k odpor

float temperatures[20] = {0};

void temperature_read_task(void *)
{
  while (true)
  {
    for (uint8_t i = 0; i < temperatureSensors.getDeviceCount(); i++)
    {
      temperatures[i] = temperatureSensors.getTempCByIndex(i);
      vTaskDelay(pdMS_TO_TICKS(5));
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void temp_read_begin()
{
  temperatureSensors.begin();
  uint8_t number_of_sensors = temperatureSensors.getDeviceCount();

  if (number_of_sensors == 0)
  {
#ifdef DEBUG
    Serial.printf("[Temp Reader] No temperature sensors detected\r\n");
#endif
    return;
  }

#ifdef DEBUG
  Serial.printf("[Temp Reader] Found %d temperature probes\r\n");
#endif

  xTaskCreateUniversal(temperature_read_task, "temp_read", 2048, NULL, 1, NULL, 0);
}

uint8_t temp_reader_sensor_count()
{
  return temperatureSensors.getDeviceCount();
}
