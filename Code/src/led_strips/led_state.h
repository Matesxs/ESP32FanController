#pragma once

#include <NeoPixelBus.h>

#include "effects/base_effect.h"

#define MAX_LED_COUNT 100

class LedState
{
public:
  RgbColor values[MAX_LED_COUNT] = {{0, 0, 0}};
  int count = 0;
  bool dirty = false;
  NeoPixelBus<NeoGrbFeature, NeoWs2811Method> &pixels;
  BaseEffect *function = 0;
  
  LedState(NeoPixelBus<NeoGrbFeature, NeoWs2811Method> &ledPixels)
  :pixels(ledPixels)
  {
    count = pixels.PixelCount();
  }

  void setFunction(BaseEffect *newFunction)
  {
    if(function)
      delete function;
    function = newFunction;
    if(!function)
      return;
    function->state = this;
  }

  void setRgb(int i, uint8_t r, uint8_t g, uint8_t b)
  {
    values[i][0] = r;
    values[i][1] = g;
    values[i][2] = b;
    dirty = true;
  }

  void setRgb(int i, RgbColor color)
  {
    values[i] = color;
    dirty = true;
  }

  RgbColor getColorOfPixel(int index)
  {
    return pixels.GetPixelColor(index);
  }

  virtual void render()
  {
    if(function)
      function->render();
  }

  void setValues(LedState &to)
  {
    for(int i = 0; i < count; i++)
      for(int j = 0; j < 3; j++)
        values[i][j] = to.values[i][j];
    setFunction(to.function);
    to.function = 0;
    dirty = true;
  }

  void commit()
  {
    if(!dirty)
      return;
      
    for(int i = 0; i < count; i++)      
      pixels.SetPixelColor(i, values[i]);

    dirty = false;
  }

  void fade(LedState &to, double prog)
  {
    for(int i = 0; i < count; i++)
    {
      RgbColor targetColor(RgbColor::LinearBlend(values[i], to.values[i], static_cast<uint8_t>(prog * 255)));
      if (pixels.GetPixelColor(i) != targetColor)
        pixels.SetPixelColor(i, targetColor);
    }

    dirty = true;
  }
};
