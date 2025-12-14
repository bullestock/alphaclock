#pragma once

#include <string>
#include <vector>

#include <driver/gpio.h>

constexpr const char* VERSION = "0.0.1";

// NVS keys
constexpr const char* WIFI_KEY = "wifi";
constexpr const char* CALIBRATION_KEY = "cal";
constexpr const char* MOTOR_DEBUG_KEY = "mdb";
constexpr const char* MOTOR_DELAY_KEY = "mde";

// Pin definitions
constexpr const auto PIN_I2S_DATA = (gpio_num_t) 21;
constexpr const auto PIN_I2S_BCK  = (gpio_num_t) 16;
constexpr const auto PIN_I2S_WS   = (gpio_num_t) 17;

constexpr const char* TAG = "ACLK";

constexpr const int MOTOR_COUNT = 3;

enum Mode {
    MODE_MANUAL,
    MODE_NORMAL,
    MODE_FAST,
};

enum HourMode {
    HOUR_MODE_DISCRETE,
    HOUR_MODE_CONTINUOUS,
};

using wifi_creds_t = std::vector<std::pair<std::string, std::string>>;

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:
