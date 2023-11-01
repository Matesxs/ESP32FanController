#pragma once

#include <Arduino.h>
#include <limits.h>
#include <algorithm>

#include "mic/utils.h"
#include "lerper.h"

typedef struct
{
  uint8_t tachoPin;
  uint8_t pwmPin;
  uint8_t pwmChannel;
} FanPins;

typedef struct
{
  #define NUMBER_OF_CURVE_POINTS 20

  typedef struct
  {
    double temperature;
    double powerSetting;

    void set(double x, double y)
    {
      temperature = x;
      powerSetting = y;
    }
  } CurvePoint;

  uint64_t tempSource; // Bit for each source
  CurvePoint points[NUMBER_OF_CURVE_POINTS];

  void setConstant(double value)
  {
    for (size_t i = 0; i < NUMBER_OF_CURVE_POINTS; i++)
      points[i].set(0, value);
  }

  double eval(double temp)
  {
    if (tempSource == 0) return points[0].powerSetting; // No source set so temperature passed will be 0

    if (temp <= points[0].temperature)
      return points[0].powerSetting;
    else if (temp >= points[NUMBER_OF_CURVE_POINTS - 1].temperature)
      return points[NUMBER_OF_CURVE_POINTS - 1].powerSetting;

    size_t i = 1;
    for (; i < NUMBER_OF_CURVE_POINTS; i++)
    {
      if (points[i].temperature == temp)
        return points[i].powerSetting;
      else if (points[i].temperature > temp)
        break;
    }

    return map(temp, points[i - 1].temperature, points[i].temperature, points[i - 1].powerSetting, points[i].powerSetting);
  }

  void getCurve(double curve_temps[], double curve_setpoints[])
  {
    for (size_t i = 0; i < NUMBER_OF_CURVE_POINTS; i++)
    {
      curve_temps[i] = points[i].temperature;
      curve_setpoints[i] = points[i].powerSetting;
    }
  }

  void sort()
  {
    std::sort(points, points + NUMBER_OF_CURVE_POINTS, 
              [](CurvePoint const & a, CurvePoint const & b) -> bool
              { return a.temperature < b.temperature; });
  }
} TemperatureCurve;

typedef struct
{
  bool manual;
  bool dirty;
  double currentPower;
  double _powerTemperature;
  uint16_t rpm;
  
  TemperatureCurve curve;
  Lerper<double> lerper;
} FanData;
