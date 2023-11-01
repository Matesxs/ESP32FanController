#pragma once

#include "mic/datastores.h"

const FanPins FAN_PINS_HOLDER[] = {
  {36, 25, 0},
  {39, 26, 1},
  {34, 27, 2},
  {35, 14, 3},
  {32, 12, 4},
  {33, 13, 5},
  {17, 15, 6},
  {18, 2, 7},
  {19, 4, 8},
  {21, 5, 9}
};

const size_t FAN_COUNT = sizeof(FAN_PINS_HOLDER) / sizeof(FAN_PINS_HOLDER[0]);

#define ONE_WIRE_BUS 16
#define LED_STRIP1_DATA_PIN 22
#define LED_STRIP2_DATA_PIN 23
