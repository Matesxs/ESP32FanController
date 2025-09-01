#pragma once

#include <cstdint>
#include <hal/ledc_types.h>

namespace PWMControl
{
  uint32_t setupTimer(uint8_t timer, uint8_t resolution, uint32_t freq);

  uint32_t readFreq(uint8_t timer);
  uint8_t getResolution(uint8_t timer);
  uint32_t getMaxValue(uint8_t timer);

  void attachPin(uint8_t timer, uint8_t channel, uint8_t pin, bool invert=false);
  void detachPin(uint8_t pin);

  uint32_t read(uint8_t channel);
  void write(uint8_t channel, uint32_t duty);
}
