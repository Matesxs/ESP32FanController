#pragma once

#include <NeoPixelBus.h>

#include "base_effect.h"
#include "led_strips/led_state.h"

class RainbowFunction: public BaseEffect
{
public:
  RainbowFunction(uint8_t darken=255) :
    m_darken(darken)
  {
  }

  virtual void render()
  {
    int j = (millis() >> 8) & 255;
    for(int i = 0; i < state->count; i++)
    {
      int ij = 255 - ((i + j) & 255);
      if(ij < 85)
        state->setRgb(i, RgbColor(255 - ij * 3, 0, ij * 3).Dim(m_darken));
      else if(ij < 170)
        state->setRgb(i, RgbColor(0, (ij - 85) * 3, 255 - (ij - 85) * 3).Dim(m_darken));
      else
        state->setRgb(i, RgbColor(((ij -170) * 3, 255 - (ij -170) * 3, 0)).Dim(m_darken));
    }
  }

private:
  uint8_t m_darken;
};
