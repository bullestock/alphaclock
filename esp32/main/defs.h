#pragma once

#include <string>
#include <vector>

#include <driver/gpio.h>

constexpr const char* VERSION = "0.0.1";

// NVS keys
constexpr const char* WIFI_KEY = "wifi";

// Pin definitions
constexpr const auto PIN_A1 = (gpio_num_t) 16;
constexpr const auto PIN_A2 = (gpio_num_t) 17;
constexpr const auto PIN_B1 = (gpio_num_t) 23;
constexpr const auto PIN_B2 = (gpio_num_t) 5;
constexpr const auto PIN_EN = (gpio_num_t) 26;

constexpr const char* TAG = "ACLK";

using wifi_creds_t = std::vector<std::pair<std::string, std::string>>;
