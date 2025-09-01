#pragma once

#include <Arduino.h>

String getValue(String data, char separator, int index);
String joinStrings(const String* strings, size_t number_of_strings, char separator=';');
String formatStringArgs(const char* format, va_list args);
String formatString(const char *format, ...);
String stripString(String string);
