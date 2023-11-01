#pragma once

#include "utils.h"

#ifdef ENABLE_DEBUG
#define DEBUG(...) Serial.printf("%s at %d\t%s", __FILE__, __LINE__, format_string(__VA_ARGS__).c_str())
#else
#define DEBUG(...)
#endif
