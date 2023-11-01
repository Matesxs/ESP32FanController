#pragma once

#include <Arduino.h>

#define GET_BIT(val, n) ((val >> n) & 1UL)

String getSubstring(String data, char separator, int index);

float map(float x, float in_min, float in_max, float out_min, float out_max);
double map(double x, double in_min, double in_max, double out_min, double out_max);

String format_string(const char *format, ...);

float temperatureFromSource(uint64_t source);
