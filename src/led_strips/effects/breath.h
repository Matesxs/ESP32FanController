#pragma once

#include <NeoPixelBus.h>

#include "lerper.h"
#include "mic/utils.h"
#include "base_effect.h"
#include "led_strips/led_state.h"

class Breath: public BaseEffect
{
public:
  Breath(uint32_t duration=10000) :
    m_lerpProgress(0.0),
    m_duration(duration)
  {
    m_lerper.setReference(&m_lerpProgress);
    m_lerper.set(255, duration);
  }

  Breath(uint8_t *color, uint32_t duration=10000) :
    m_lerpProgress(0.0),
    m_duration(duration)
  {
    for(int i = 0; i < 3; i++)
      rgb[i] = color[i];

    m_lerper.setReference(&m_lerpProgress);
    m_lerper.set(255, duration);
  }
  
  virtual void render()
  {
    m_lerper.update();

    for(int i = 0; i < state->count; i++)
      state->setRgb(i, rgb.Dim(m_lerpProgress));

    if (m_lerper.finished())
    {
      if (m_lerpProgress == 0)
        m_lerper.set(255, m_duration);
      else
        m_lerper.set(0, m_duration);
    }
  }

private:
  uint32_t m_duration;
  RgbColor rgb = {0, 0, 0};
  Lerper<uint8_t> m_lerper;
  uint8_t m_lerpProgress;
};
