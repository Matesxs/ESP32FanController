#include "utils.h"

#include "data_share.h"
#include "temperature_reader.h"

String getSubstring(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

float map(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

double map(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

String format_string(const char *format, ...)
{
  char loc_buf[32];
  char *temp = loc_buf;
  va_list arg;
  va_list copy;

  va_start(arg, format);
  va_copy(copy, arg);
  int len = vsnprintf(temp, sizeof(loc_buf), format, copy);
  va_end(copy);

  if (len < 0)
  {
    va_end(arg);
    return String();
  };

  if (len >= sizeof(loc_buf))
  {
    temp = (char *)malloc(len + 1);
    if (temp == NULL)
    {
      va_end(arg);
      return String();
    }

    len = vsnprintf(temp, len + 1, format, arg);
  }
  va_end(arg);

  String outputBuffer(temp);

  if (temp != loc_buf)
    free(temp);

  return outputBuffer;
}

float temperatureFromSource(uint64_t source)
{
  float sourceTemp = 0.0f;
  if (GET_BIT(source, 0))
    sourceTemp = externalTemperature;

  for (uint8_t i = 0; i < temp_reader_sensor_count(); i++)
  {
    if (GET_BIT(fanData[i].curve.tempSource, i + 1))
      sourceTemp = max(sourceTemp, temperatures[i]);
  }

  return sourceTemp;
}
