//
// Created by Martin on 6. 7. 2025.
//

#include <lerper.h>
#include <pwmControl.h>
#include <esp_task_wdt.h>

#include "pins.h"
#include "settings.h"
#include "datastores.h"
#include "custom_debug.h"
#include "helpers.h"

static uint64_t lastYield = 0;

extern uint32_t avgFanRPM[FAN_COUNT];
extern TemperatureData temperatureData[MAXIMUM_TEMPERATURE_SENSORS];
extern FanControlData fanControlData[PWM_CHANNELS_COUNT];
extern float externalTemperature;

float getMaximalTemperature(const std::bitset<MAXIMUM_TEMPERATURE_SENSORS + 1> sourceSelect)
{
  float temperature = MINIMAL_TEMPERATURE_C;

  for (size_t i = 0; i < MAXIMUM_TEMPERATURE_SENSORS; i++)
  {
    if (sourceSelect.test(i))
    {
      if (temperatureData[i].valid)
        temperature = max(temperature, temperatureData[i].temperature);
      else
        temperature = MAXIMAL_TEMPERATURE_C;
    }
  }

  // If last bit is selected then use external temperature
  if (sourceSelect.test(MAXIMUM_TEMPERATURE_SENSORS)) temperature = max(temperature, externalTemperature);

  return temperature;
}

uint32_t percToPwm(const double percent)
{
  static const double fanPwmStep = static_cast<double>(FAN_PWM_MAX_VALUE) / 100.0;

  if (percent == 0.0) return 0;
  return min(static_cast<uint32_t>(percent * fanPwmStep), FAN_PWM_MAX_VALUE);
}

double pwmToPerc(const uint32_t pwm)
{
  static const double fanPwmStep = static_cast<double>(FAN_PWM_MAX_VALUE) / 100.0;

  if (pwm == 0) return 0.0;
  return min(static_cast<double>(pwm) / fanPwmStep, 100.0);
}

double mapTemperatureToPower(const float temperature, const float min_temp, const float max_temp, const double min_pow, const double max_pow)
{
  if (temperature <= min_temp)
    return min_pow;

  if (temperature >= max_temp)
    return max_pow;

  // Calculate the range of temperatures and power values
  const float temp_range = max_temp - min_temp;
  const double pow_range = max_pow - min_pow;

  return static_cast<double>((temperature - min_temp) / temp_range) * pow_range + min_pow;
}

void calculateFanRpm(FanControlData& fan)
{
  std::lock_guard lock(fan._lock);

  switch (fan.state)
  {
    case FS_STOPPED:
      if (fan.pwmData.temperature >= fan.settings.startTemperature)
      {
        fan.pwmData.targetPwm = percToPwm(fan.settings.startPower);
        fan.state = FS_START_BUMP;
        fan.pwmData.counter = millis();
      }
      break;

    case FS_START_BUMP:
      if (millis() - fan.pwmData.counter >= FAN_POWERON_BUMP_DURATION_MS)
        fan.state = FS_RUNNING;
      break;

    case FS_RUNNING:
      if (fan.pwmData.temperature < fan.settings.minimalTemperature)
      {
        fan.pwmData.targetPwm = 0;
        fan.state = FS_STOPPED;
      }
      else
      {
        // LOG_DEBUG("Current temperature %.1f°C, min temp: %.1f°C, max temp: %.1f°C, min pow: %.1f%%, max pow: %.1f%%", fan.pwmData.temperature, fan.settings.minimalTemperature, fan.settings.maximalTemperature, fan.settings.minimalPower, fan.settings.maximalPower);
        const double mappedPower = mapTemperatureToPower(fan.pwmData.temperature, fan.settings.minimalTemperature, fan.settings.maximalTemperature, fan.settings.minimalPower, fan.settings.maximalPower);
        // LOG_DEBUG("Mapped power: %.1f", mappedPower);
        fan.pwmData.targetPwm = percToPwm(mappedPower);
      }
      break;

    default:
      break;
  }
}

void manageFanPwm(const uint8_t channel, FanControlData& fan)
{
  const uint32_t currentSetPwm = PWMControl::read(channel);
  if (fan.pwmData.targetPwm != fan.pwmData.lerper.getTarget())
  {
    if (fan.state == FS_START_BUMP || fan.state == FS_STOPPED || fan.state == FS_FORCED)
      fan.pwmData.lerper.set(fan.pwmData.targetPwm, fan.pwmData.targetPwm, 0);
    else
    {
      if (currentSetPwm < fan.pwmData.targetPwm)
        fan.pwmData.lerper.set(fan.pwmData.targetPwm, fan.settings.spoolUpTime);
      else
        fan.pwmData.lerper.set(fan.pwmData.targetPwm, fan.settings.spoolDownTime);
    }
  }

  if (fan.state != FS_START_BUMP && fan.state != FS_STOPPED && fan.state != FS_FORCED)
    fan.pwmData.currentPwm = fan.pwmData.lerper.update();
  else
    fan.pwmData.currentPwm = fan.pwmData.targetPwm;

  if (fan.pwmData.currentPwm != currentSetPwm)
    PWMControl::write(channel, fan.pwmData.currentPwm);
}

[[noreturn]] void manageFansTask(void*)
{
  if (esp_task_wdt_add(nullptr) != ESP_OK)
  {
    LOG_ERROR("Failed to add fan management task to WDT");
  }

  for (const uint8_t channel : PWM_CHANNELS)
    PWMControl::write(channel, 0);
  esp_task_wdt_reset();

  while (true)
  {
    for (size_t i = 0; i < PWM_CHANNELS_COUNT; i++)
    {
      // Get latest temperature for current fan
      fanControlData[i].pwmData.temperature = getMaximalTemperature(fanControlData[i].settings.temperatureSourceSelect);
      yieldIfNecessary(lastYield);
      esp_task_wdt_reset();

      if (fanControlData[i].settings.enabled)
      {
        calculateFanRpm(fanControlData[i]);
        yieldIfNecessary(lastYield);
        esp_task_wdt_reset();
      }

      // Even when channel is disabled manage pwm here
      manageFanPwm(PWM_CHANNELS[i], fanControlData[i]);
      yieldIfNecessary(lastYield);
      esp_task_wdt_reset();
    }

    vTaskDelay(pdMS_TO_TICKS(TEMPERATURE_CONTROL_POLL_INTERVAL_MS));
    esp_task_wdt_reset();
  }
}

void temeperatureControllerInit()
{
  xTaskCreateUniversal(manageFansTask, "temp_cntrl", 2048, nullptr, 1, nullptr, 1);
}
