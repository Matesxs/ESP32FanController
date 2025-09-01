#pragma once

#include <cstdint>

typedef struct
{
  uint8_t tachoPin;
  uint8_t pwmPin;
  uint8_t pwmChannel;
} FanPins;
