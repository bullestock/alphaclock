#pragma once

#include "defs.h"

class Stepper
{
public:
    Stepper();

    void step(int nof_steps, uint64_t delay_us);

private:
    bool timer_enabled = false;
};
