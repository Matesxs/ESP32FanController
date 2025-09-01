#pragma once

#if DEBUG_LEVEL != 0

#include "string_helpers.h"

#define _PRINTNL if (Serial) Serial.println() // NOLINT(*-reserved-identifier)

#if DEBUG_LEVEL >= 3
#define DEBUG_ENABLED
#define DEBUGNL() _PRINTNL
#ifdef DEBUG_LOCATION
#define LOG_DEBUG(...) if (Serial) Serial.printf("[DEBUG] %s:%d\t%s\r\n", __FILE__, __LINE__, stripString(formatString(__VA_ARGS__)).c_str())
#else
#define LOG_DEBUG(...) if (Serial) Serial.printf("[DEBUG] %s\r\n", stripString(formatString(__VA_ARGS__)).c_str())
#endif
#else
#define DEBUG(...)
#define DEBUGNL()
#endif

#if DEBUG_LEVEL >= 2
#define INFO_ENABLED
#define INFONL() _PRINTNL
#ifdef DEBUG_LOCATION
#define LOG_INFO(...) if (Serial) Serial.printf("[INFO] %s:%d\t%s\r\n", __FILE__, __LINE__, stripString(formatString(__VA_ARGS__)).c_str())
#else
#define LOG_INFO(...) if (Serial) Serial.printf("[INFO] %s\r\n", stripString(formatString(__VA_ARGS__)).c_str())
#endif
#else
#define INFO(...)
#define INFONL()
#endif

#if DEBUG_LEVEL >= 1
#define ERROR_ENABLED
#define ERRORNL() _PRINTNL
#ifdef DEBUG_LOCATION
#define LOG_ERROR(...) if (Serial) Serial.printf("[ERROR] %s:%d\t%s\r\n", __FILE__, __LINE__, stripString(formatString(__VA_ARGS__)).c_str())
#else
#define LOG_ERROR(...) if (Serial) Serial.printf("[ERROR] %s\r\n", stripString(formatString(__VA_ARGS__)).c_str())
#endif
#else
#define ERROR(...)
#define ERRORNL()
#endif

#else
#define LOG_DEBUG(...)
#define LOG_INFO(...)
#define LOG_ERROR(...)
#define DEBUGNL()
#define INFONL()
#define ERRORNL()
#endif