#include "data_share.h"

#include "settings.h"

Preferences mainSettings;
Preferences curvesStore;
FanData fanData[FAN_COUNT] = { 0 };
float externalTemperature = 0.0f;
#ifdef LED_STRIPS_ENABLE
StripHandler* ledStripChannel1 = NULL;
StripHandler* ledStripChannel2 = NULL;
#endif
