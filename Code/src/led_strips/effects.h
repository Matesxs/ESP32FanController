#pragma once

#include <Arduino.h>

#include "led_strips/effects/solid_color.h"
#include "led_strips/effects/wave.h"
#include "led_strips/effects/rainbow.h"
#include "led_strips/effects/kitt.h"
#include "led_strips/effects/breath.h"

BaseEffect* effectFromString(String string);
