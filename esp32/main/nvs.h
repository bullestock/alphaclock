#pragma once

#include "defs.h"

struct calibration_data
{
    uint8_t motor;
    uint8_t reverse;
    double steps;
};

void init_nvs();

wifi_creds_t get_wifi_creds();

void clear_wifi_credentials();
void add_wifi_credentials(const char* ssid, const char* password);

void set_calibration(int motor, int reverse, double steps);

const calibration_data& get_calibration(int motor);

void set_motor_debug(bool on);

bool get_motor_debug();

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:

