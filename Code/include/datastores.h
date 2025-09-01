#pragma once

#include <bitset>
#include <mutex>

#include "lerper.h"

#include "settings.h"

struct FanPins
{
  uint8_t tachoPin;
  uint8_t pwmPin;
  uint8_t pwmTimer;
  uint8_t pwmChannel;
  uint8_t invert:1;
};

struct TemperatureData
{
  uint8_t valid:1;
  float temperature;
};

enum FanState
{
  FS_STOPPED,
  FS_START_BUMP,
  FS_RUNNING,
  FS_FORCED,
};

inline const char* fanStateToString(const FanState state)
{
  switch (state)
  {
    case FS_STOPPED: return "STOPPED";
    case FS_START_BUMP: return "START_BUMP";
    case FS_RUNNING: return "RUNNING";
    case FS_FORCED: return "FORCED";
  }

  return "UNKNOWN";
}

struct FanSettings
{
  uint8_t enabled:1 = false;
  float startTemperature = MINIMAL_TEMPERATURE_C;
  float minimalTemperature = MINIMAL_TEMPERATURE_C;
  float maximalTemperature = MAXIMAL_TEMPERATURE_C;
  double startPower = 100.0;
  double minimalPower = 100.0;
  double maximalPower = 100.0;
  uint32_t spoolUpTime = 1000;
  uint32_t spoolDownTime = 1000;
  std::bitset<MAXIMUM_TEMPERATURE_SENSORS + 1> temperatureSourceSelect;
};

struct FanPWMData
{
  uint32_t currentPwm = 0;
  uint32_t targetPwm = 0;
  float temperature = MINIMAL_TEMPERATURE_C;
  uint32_t counter = 0;
  Lerper<uint32_t> lerper;
};

struct FanControlData
{
  FanState state = FS_STOPPED;
  FanSettings settings;
  FanPWMData pwmData;
  std::mutex _lock; // Guard for fan state, used mainly for switching to and from forced mode
};

