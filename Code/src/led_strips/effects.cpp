#include "effects.h"

#include <sstream>

#include "mic/utils.h"

BaseEffect *effectFromString(String string)
{
  String mode = getSubstring(string, '@', 0);
  String modeSettingsString = getSubstring(string, '@', 1);
  BaseEffect* selectedEffect = NULL;

  if (mode == "off")
  {
    selectedEffect = static_cast<BaseEffect*>(new SolidColor());
  }
  else if (mode == "solid" && modeSettingsString != "")
  {
    uint8_t color[3];
    for (size_t i = 0; i < 3; i++)
      color[i] = static_cast<uint8_t>(min(static_cast<long>(255), max(static_cast<long>(0), getSubstring(modeSettingsString, ';', i).toInt())));

    selectedEffect = static_cast<BaseEffect*>(new SolidColor(color));
  }
  else if (mode == "wave" && modeSettingsString != "")
  {
    uint8_t color[3];
    for (size_t i = 0; i < 3; i++)
      color[i] = static_cast<uint8_t>(min(static_cast<long>(255), max(static_cast<long>(0), getSubstring(modeSettingsString, ';', i).toInt())));

    selectedEffect = static_cast<BaseEffect*>(new WaveEffect(color));
  }
  else if (mode == "tsolid" && modeSettingsString != "")
  {
    String sourceString = getSubstring(modeSettingsString, ';', 0);
    String minTemperatureString = getSubstring(modeSettingsString, ';', 1);
    String maxTemperatureString = getSubstring(modeSettingsString, ';', 2);
    float minTemperature = minTemperatureString.toFloat();
    float maxTemperature = maxTemperatureString.toFloat();

    uint64_t source;
    std::istringstream iss(sourceString.c_str());
    iss >> source;

    if (sourceString != "" && minTemperatureString != "" && maxTemperatureString != "" &&
        !isnan(source) &&
        !isnan(minTemperature) &&
        !isnan(maxTemperature) &&
        minTemperature <= maxTemperature)
    {
      selectedEffect = static_cast<BaseEffect*>(new TemperatureSolidColor(source, minTemperature, maxTemperature));
    }
  }
  else if (mode == "twave" && modeSettingsString != "")
  {
    String sourceString = getSubstring(modeSettingsString, ';', 0);
    String minTemperatureString = getSubstring(modeSettingsString, ';', 1);
    String maxTemperatureString = getSubstring(modeSettingsString, ';', 2);
    float minTemperature = minTemperatureString.toFloat();
    float maxTemperature = maxTemperatureString.toFloat();

    uint64_t source;
    std::istringstream iss(sourceString.c_str());
    iss >> source;

    if (sourceString != "" && minTemperatureString != "" && maxTemperatureString != "" &&
        !isnan(source) &&
        !isnan(minTemperature) &&
        !isnan(maxTemperature) &&
        minTemperature <= maxTemperature)
    {
      selectedEffect = static_cast<BaseEffect*>(new TemperatureWaveEffect(source, minTemperature, maxTemperature));
    }
  }
  else if (mode == "rainbow")
  {
    if (modeSettingsString != "")
    {
      uint8_t darken = static_cast<uint8_t>(min(static_cast<long>(255), max(static_cast<long>(0), modeSettingsString.toInt())));
      if (!isnan(darken))
        selectedEffect = static_cast<BaseEffect*>(new RainbowFunction(darken));
    }
    else
      selectedEffect = static_cast<BaseEffect*>(new RainbowFunction());
  }
  else if (mode == "kitt" && modeSettingsString != "")
  {
    uint8_t color[3];
    for (size_t i = 0; i < 3; i++)
      color[i] = static_cast<uint8_t>(min(static_cast<long>(255), max(static_cast<long>(0), getSubstring(modeSettingsString, ';', i).toInt())));

    selectedEffect = static_cast<BaseEffect*>(new Kitt(color));
  }
  else if (mode == "breath" && modeSettingsString != "")
  {
    String colorString = getSubstring(modeSettingsString, '&', 0);
    String durationString = getSubstring(modeSettingsString, '&', 1);

    uint8_t color[3];
    for (size_t i = 0; i < 3; i++)
      color[i] = static_cast<uint8_t>(min(static_cast<long>(255), max(static_cast<long>(0), getSubstring(colorString, ';', i).toInt())));

    if (durationString != "")
    {
      uint32_t duration = static_cast<uint32_t>(durationString.toInt());
      if (!isnan(duration))
        selectedEffect = static_cast<BaseEffect*>(new Breath(color, duration));
    }
    else
      selectedEffect = static_cast<BaseEffect*>(new Breath(color));
  }

  return selectedEffect;
}
