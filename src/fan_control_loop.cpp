#include "fan_control_loop.h"

#include <Arduino.h>

#include "pins.h"
#include "mic/data_share.h"
#include "settings.h"
#include "temperature_reader.h"
#include "mic/debug.h"

void fan_speed_control_task(void *)
{
  while (true)
  {
    for (size_t i = 0; i < FAN_COUNT; i++)
    {
      if (!fanData[i].manual)
      {
        float sourceTemp = temperatureFromSource(fanData[i].curve.tempSource);
        
        if (fanData[i].dirty || abs(fanData[i]._powerTemperature - sourceTemp) > TEMPERATURE_HYSTERESIS)
        {
          double targetPower = fanData[i].curve.eval(sourceTemp);
          fanData[i].lerper.set(targetPower, fanData[i].dirty ? FAN_SPEED_RAMP_TIME_DIRTY_MS : FAN_SPEED_RAMP_TIME_MS);

          DEBUG("[Fan Temperature Control] Evaluating temperature curve for fan %d to %.2f target power\r\n", i, targetPower);

          fanData[i].dirty = false;
          fanData[i]._powerTemperature = sourceTemp;
        }
      }

      if (!fanData[i].lerper.finished())
      {
        auto pinSettings = FAN_PINS_HOLDER[i];
        fanData[i].lerper.update();

        auto pwmValue = static_cast<uint32_t>(map(fanData[i].currentPower, 0.0, 100.0, static_cast<double>(MAX_FAN_PWM_VALUE), 0.0));
        DEBUG("[Fan Speed Control] Setting speed for fan %d to %.2f (%d)\r\n", i, fanData[i].currentPower, pwmValue);

        ledcWrite(pinSettings.pwmChannel, pwmValue);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void begin_fan_control()
{
  xTaskCreateUniversal(fan_speed_control_task, "fan_contrl", 4096, NULL, 1, NULL, 0);
}
