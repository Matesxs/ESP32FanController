//
// Created by Martin on 5. 7. 2025.
//

#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

// Features
#define ENABLE_WIFI
#define ENABLE_OTA

// Fan PWM settings
#define FAN_PWM_RESOLUTION 12
#define FAN_PWM_FREQUENCY_HZ 10000

const uint32_t FAN_PWM_MAX_VALUE = static_cast<uint32_t>(pow(2, FAN_PWM_RESOLUTION) - 1);

// Fan tacho settings
#define SAMPLES_PER_RPM_READING 10
#define FAN_PULSES_PER_ROTATION 2
#define FAN_NUMBER_OF_POLES 1
#define FAN_SPEED_AVERAGE_SAMPLES 3
#define FAN_SPEED_SAMPLING_INTERVAL_MS 500

// Temperature sensing
#define MINIMAL_TEMPERATURE_C (-100.0f)
#define MAXIMAL_TEMPERATURE_C (255.0f)
#define MAXIMUM_TEMPERATURE_SENSORS 31
#define TEMPERATURE_AVERAGE_SAMPLES 10
#define TEMPERATURE_SAMPLING_INTERVAL_MS 1000

// Temperature control
#define FAN_POWERON_BUMP_DURATION_MS 1000
#define FAN_POWERON_RETRY_TIMEOUT_MS 3000
#define TEMPERATURE_CONTROL_POLL_INTERVAL_MS 100

// Uart communication
#define UART_POLL_INTERVAL_MS 20

// WiFi
#define DEFAULT_HOSTNAME "esp32_fan_controller"

// Webserver
#define WEBSERVER_UPDATE_INTERVAL_MS 5000

#endif //SETTINGS_H
