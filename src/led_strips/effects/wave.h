#pragma once

#include <NeoPixelBus.h>

#include "lerper.h"
#include "mic/utils.h"
#include "base_effect.h"
#include "led_strips/led_state.h"

class WaveEffect: public BaseEffect
{
public:
  long sinTab[256];
  RgbColor rgb = {0, 0, 0};

  WaveEffect()
  {
    for(int i = 0; i < 256; i++)
      sinTab[i] = sin(3.1415 / 128 * i) * 0x7fff + 0x8000;
  }

  WaveEffect(uint8_t *color)
  {
    for(int i = 0; i < 256; i++)
      sinTab[i] = sin(3.1415 / 128 * i) * 0x7fff + 0x8000;

    for(int i = 0; i < 3; i++)
      rgb[i] = color[i];
  }
  
  virtual void render()
  {
    int j = ((millis() - start) / 63) & 255;
    int k = ((millis() - start) / 71) & 255;
    for(int i = 0; i < state->count; i++)
    {
      long s = (sinTab[(i*3 + j) & 255] >> 8) * (sinTab[-(i*4 + k) & 255] >> 8);
      state->setRgb(i, (rgb[0] * s) >> 16, (rgb[1] * s) >> 16, (rgb[2] * s) >> 16);
    }
  }
};

class TemperatureWaveEffect: public WaveEffect
{
public:
  TemperatureWaveEffect(uint64_t temperature_source, float min_temp, float max_temp) :
    m_temperatureSource(temperature_source),
    m_minTemp(min_temp),
    m_maxTemp(max_temp),
    m_lerpProgress(0.0)
  {
    float temperature = temperatureFromSource(temperature_source);
    m_targetTemperature = temperature;
    rgb = RgbColor(HslColor(temperatureToHue(temperature), 1.0, 0.5));
    m_lerper.setReference(&m_lerpProgress);
  }

  float temperatureToHue(float temperature)
  {
    return map(temperature, m_minTemp, m_maxTemp, static_cast<float>(115.0 / 360.0), static_cast<float>(0.0 / 360.0));
  }

  virtual void render()
  {
    float temperature = temperatureFromSource(m_temperatureSource);
    if (m_targetTemperature != temperature)
    {
      m_targetColor = RgbColor(HslColor(temperatureToHue(temperature), 1.0, 0.5));
      m_lerper.set(1.0, 500, 0.0);
      m_targetTemperature = temperature;
    }

    if (!m_lerper.finished())
    {
      m_lerper.update();
      WaveEffect::rgb = RgbColor::LinearBlend(rgb, m_targetColor, static_cast<uint8_t>(m_lerpProgress * 255));
    }
    
    WaveEffect::render();
  }

private:
  Lerper<float> m_lerper;
  float m_lerpProgress;
  float m_minTemp;
  float m_maxTemp;
  uint64_t m_temperatureSource;
  float m_targetTemperature;
  RgbColor m_targetColor = {0, 0, 0};
};
