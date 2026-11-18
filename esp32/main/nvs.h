#pragma once

#include "defs.h"

struct calibration_data
{
    // should we run the motor in reverse to make the hand go clockwise?
    uint8_t reverse;
    // number of steps per revolution
    double steps;
    // After homing, this is where the hand will be (in "minutes"). Corresponds to motor absolute zero.
    double sensor_home_position;
    // This is when the sensor is no longer activated when going from motor absolute zero. In steps.
    int sensor_end_position;
};

void init_nvs();

wifi_creds_t get_wifi_creds();

void clear_wifi_credentials();
void add_wifi_credentials(const char* ssid, const char* password);

void set_calibration(int motor, int reverse, double steps,
                     double sensor_home_position, int sensor_end_position);

const calibration_data& get_calibration(int motor);

void set_motor_delay(int delay);

int get_motor_delay();

void set_motor_debug(int motor, bool on);

bool get_motor_debug(int motor);

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:

