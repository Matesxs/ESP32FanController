#pragma once

#include <Arduino.h>

// Functions settings
// #define USE_ARGUS_INTERFACE
#define COM_SERIAL Serial

#define WIFI_CONNECTION
#define WIFI_CONTROL
#define CUSTOM_DNS
#define DEFAULT_DNS_NAME "esp32_fan_controler"
#define LED_STRIPS_ENABLE

// Fan pwm settings
#define FAN_PWM_FREQ 1000
#define FAN_PWM_RES 10
const uint16_t MAX_FAN_PWM_VALUE = pow(2, FAN_PWM_RES) - 1;
const double DEFAULT_PWM_PERCENT = 25.0;

// Tacho settings
#define SAMPLES_PER_RPM_READING 5
#define FAN_PULSES_PER_ROTATION 2
#define FAN_NUMBER_OF_POLES 1

// Fan Control loop
#define FAN_SPEED_RAMP_TIME_DIRTY_MS 1000
#define FAN_SPEED_RAMP_TIME_MS 5000
#define TEMPERATURE_HYSTERESIS 2.0

// Mic
#define ARGUS_DEVICE_ID 1
