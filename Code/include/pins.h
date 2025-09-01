#pragma once

#include "datastores.h"

#include <soc/soc_caps.h>

#if SOC_LEDC_SUPPORT_HS_MODE==1
constexpr uint8_t PWM_TIMERS[] = { 0, 4 };
constexpr uint8_t PWM_CHANNELS[] = { 0, 1, 2, 3, 4, 8, 9, 10, 11, 12 };

const FanPins FAN_PINS_HOLDER[] = {
  {36, 25, PWM_TIMERS[0], PWM_CHANNELS[0], true},
  {39, 26, PWM_TIMERS[0], PWM_CHANNELS[1], true},
  {34, 27, PWM_TIMERS[0], PWM_CHANNELS[2], true},
  {35, 14, PWM_TIMERS[0], PWM_CHANNELS[3], true},
  {32, 12, PWM_TIMERS[0], PWM_CHANNELS[4], true},
  {33, 13, PWM_TIMERS[1], PWM_CHANNELS[5], true},
  {17, 15, PWM_TIMERS[1], PWM_CHANNELS[6], true},
  {18, 2, PWM_TIMERS[1], PWM_CHANNELS[7], true},
  {19, 4, PWM_TIMERS[1], PWM_CHANNELS[8], true},
  {21, 5, PWM_TIMERS[1], PWM_CHANNELS[9], true}
};

const size_t FAN_INDEX_TO_PWM_CHANNEL_INDEX[] = {
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
};
#else
constexpr uint8_t PWM_TIMERS[] = { 0 };
constexpr uint8_t PWM_CHANNELS[] = { 0, 1, 2, 3, 4 };

const FanPins FAN_PINS_HOLDER[] = {
  {36, 25, PWM_TIMERS[0], PWM_CHANNELS[0], true},
  {39, 26, PWM_TIMERS[0], PWM_CHANNELS[1], true},
  {34, 27, PWM_TIMERS[0], PWM_CHANNELS[2], true},
  {35, 14, PWM_TIMERS[0], PWM_CHANNELS[3], true},
  {32, 12, PWM_TIMERS[0], PWM_CHANNELS[4], true},
  {33, 13, PWM_TIMERS[0], PWM_CHANNELS[0], true},
  {17, 15, PWM_TIMERS[0], PWM_CHANNELS[1], true},
  {18, 2, PWM_TIMERS[0], PWM_CHANNELS[2], true},
  {19, 4, PWM_TIMERS[0], PWM_CHANNELS[3], true},
  {21, 5, PWM_TIMERS[0], PWM_CHANNELS[4], true}
};

const size_t FAN_INDEX_TO_PWM_CHANNEL_INDEX[] = {
  0,
  1,
  2,
  3,
  4,
  0,
  1,
  2,
  3,
  4,
};
#endif

constexpr size_t PWM_CHANNELS_COUNT = std::size(PWM_CHANNELS);
constexpr size_t FAN_COUNT = std::size(FAN_PINS_HOLDER);

#define ONE_WIRE_BUS 16
#define LED_STRIP1_DATA_PIN 22
#define LED_STRIP2_DATA_PIN 23
