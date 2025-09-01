#include "string_helpers.h"

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length();

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

String joinStrings(const String* strings, size_t number_of_strings, char separator)
{
  if (number_of_strings == 0) return String();

  String outputString = String(strings[0]);
  for (size_t i = 1; i < number_of_strings; i++)
  {
    outputString += separator;
    outputString += String(strings[i]);
  }

  return outputString;
}

String formatStringArgs(const char* format, va_list args)
{
  char loc_buf[32];
  char *temp = loc_buf;

  va_list argsCopy;
  va_copy(argsCopy, args);
  int len = vsnprintf(temp, sizeof(loc_buf), format, argsCopy);
  va_end(argsCopy);

  if (len < 0)
    return String();

  if (len >= sizeof(loc_buf))
  {
    temp = (char *)malloc(len + 1);
    if (temp == nullptr)
      return String();

    len = vsnprintf(temp, len + 1, format, args);
  }

  String outputBuffer(temp);

  if (temp != loc_buf)
    free(temp);

  return outputBuffer;
}

String formatString(const char *format, ...)
{
  va_list args;

  va_start(args, format);
  String outputBuffer = formatStringArgs(format, args);
  va_end(args);

  return outputBuffer;
}

String stripString(String string)
{
  string.trim();
  return string;
}
