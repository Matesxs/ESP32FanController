#include "temperature_reader.h"

#include <Wire.h>
#include <DallasTemperature.h>

#include "settings.h"
#include "mic/debug.h"
#include "pins.h"

OneWire temperatureSensorsBus(ONE_WIRE_BUS);
DallasTemperature temperatureSensors(&temperatureSensorsBus); // 1k odpor

float temperatures[20] = {0};
uint8_t number_of_sensors;

void temperature_read_task(void *)
{
  while (true)
  {
    temperatureSensors.requestTemperatures();

    for (uint8_t i = 0; i < number_of_sensors; i++)
    {
      temperatures[i] = temperatureSensors.getTempCByIndex(i);
      DEBUG("[Temp Reader] Temp sen %d: %.3f\n", i, temperatures[i]);

      vTaskDelay(pdMS_TO_TICKS(1));
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void temp_read_begin()
{
  temperatureSensors.begin();
  number_of_sensors = min(temperatureSensors.getDeviceCount(), static_cast<uint8_t>(20));

  if (number_of_sensors == 0)
  {
    DEBUG("[Temp Reader] No temperature sensors detected\r\n");
    return;
  }

  DEBUG("[Temp Reader] Found %d temperature probes\r\n", number_of_sensors);

  xTaskCreateUniversal(temperature_read_task, "temp_read", 2048, NULL, 1, NULL, 0);
}

uint8_t temp_reader_sensor_count()
{
  return number_of_sensors;
}
