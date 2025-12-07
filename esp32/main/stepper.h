#pragma once

#include "defs.h"
#include "nvs.h"

class Stepper
{
public:
    Stepper(int enable_pin);

    int get_index() const;

    const calibration_data& get_calibration();

    void step(int nof_steps, uint64_t delay_us, bool wait = false);

    void start(bool forward, uint64_t delay_us);

    void stop();

    bool busy() const;

private:
    int motor = 0;
};

extern Stepper s_hours, s_minutes, s_seconds;
