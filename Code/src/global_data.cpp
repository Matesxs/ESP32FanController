//
// Created by Martin on 6. 7. 2025.
//

#include <Preferences.h>

#include "pins.h"
#include "datastores.h"

volatile uint8_t detectedTemperatureSensors = 0;
volatile TemperatureData temperatureData[MAXIMUM_TEMPERATURE_SENSORS];
volatile float externalTemperature = MINIMAL_TEMPERATURE_C;

volatile uint32_t currentFanSamplingTime = 0;
volatile uint32_t avgFanRPM[FAN_COUNT];
volatile FanControlData fanControlData[PWM_CHANNELS_COUNT];

volatile Preferences fanConfigs;
volatile Preferences connectionSettings;
