#pragma once

#include <Arduino.h>
#include <NeoPixelBus.h>

#include "led_state.h"
#include "fader.h"
#include "effects.h"
#include "effects/solid_color.h"

class StripHandler
{
public:
  StripHandler(uint16_t countPixels, uint8_t pin) :
    m_currentEffect("off"),
    m_length(countPixels),
    m_strip(countPixels, pin),
    m_currentLedStates(m_strip),
    m_targetLedStates(m_strip),
    m_ledFader(m_currentLedStates, m_targetLedStates)
  {
    m_strip.Begin();
    
    setTarget(m_currentEffect);
  }

  uint16_t getLength()
  {
    return m_length;
  }

  String getCurrentEffect()
  {
    return m_currentEffect;
  }

  void render()
  {
    m_currentLedStates.render();
    if(m_ledFader.isActive())
      m_targetLedStates.render();
    if(!m_ledFader.fade())
      m_currentLedStates.commit();

    m_strip.Show();
  }

  bool setTarget(String effectString)
  {
    BaseEffect* effect = effectFromString(effectString);

    if (effect != NULL)
    {
      m_targetLedStates.setFunction(effect);
      m_ledFader.start(1000);
      m_currentEffect = effectString;
      return true;
    }

    return false;
  }

private:
  String m_currentEffect;
  uint16_t m_length;
  NeoPixelBus<NeoGrbFeature, NeoWs2811Method> m_strip;
  LedState m_currentLedStates;
  LedState m_targetLedStates;
  Fader<LedState> m_ledFader;
};