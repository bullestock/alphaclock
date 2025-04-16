#pragma once

#include <string>
#include <vector>

#include <driver/gpio.h>

constexpr const char* VERSION = "0.0.1";

// NVS keys
constexpr const char* WIFI_KEY = "wifi";

// Pin definitions
constexpr const auto PIN_A1 = (gpio_num_t) 25;
constexpr const auto PIN_A2 = (gpio_num_t) 13;
constexpr const auto PIN_B1 = (gpio_num_t) 5;
constexpr const auto PIN_B2 = (gpio_num_t) 22;
constexpr const auto PIN_EN1 = (gpio_num_t) 27;
constexpr const auto PIN_EN2 = (gpio_num_t) 32;
constexpr const auto PIN_EN3 = (gpio_num_t) 12;

const bool USE_MICRO_STEPPING = true;
// Must be a multiple of 4
const int NOF_MICRO_STEPS = USE_MICRO_STEPPING ? 16 : 4;

constexpr const char* TAG = "ACLK";

using wifi_creds_t = std::vector<std::pair<std::string, std::string>>;
