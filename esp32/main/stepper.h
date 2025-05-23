#pragma once

#include "defs.h"

class Stepper
{
public:
    Stepper(int enable_pin);

    void step(int nof_steps, uint64_t delay_us, bool wait = false);

    void start(bool forward, uint64_t delay_us);

    void stop();

    bool busy() const;

private:
    int motor = 0;
};
