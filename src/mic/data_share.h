#pragma once

#include <Preferences.h>

#include "pins.h"
#include "datastores.h"
#include "led_strips/strip_handler.h"
#include "settings.h"

extern Preferences mainSettings;
extern Preferences curvesStore;
extern FanData fanData[FAN_COUNT];
extern float temperatures[20];
extern float externalTemperature;
#ifdef LED_STRIPS_ENABLE
extern StripHandler* ledStripChannel1;
extern StripHandler* ledStripChannel2;
#endif