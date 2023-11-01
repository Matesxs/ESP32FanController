#pragma once

#include <Arduino.h>

class LedState;

class BaseEffect
{  
public:
  LedState *state;
  unsigned long start;
  
  BaseEffect()
  {
    start = millis();
  }
  
  virtual void render() = 0;
};
