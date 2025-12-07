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
constexpr const auto PIN_A1 = (gpio_num_t) 25;
constexpr const auto PIN_A2 = (gpio_num_t) 13;
constexpr const auto PIN_B1 = (gpio_num_t) 5;
constexpr const auto PIN_B2 = (gpio_num_t) 22;
constexpr const auto PIN_EN1 = (gpio_num_t) 27;
constexpr const auto PIN_EN2 = (gpio_num_t) 32;
constexpr const auto PIN_EN3 = (gpio_num_t) 12;

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
