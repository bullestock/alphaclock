#pragma once

#include "defs.h"

class Stepper
{
public:
    Stepper() = default;

    void step(int nof_steps, int64_t delay_us);

private:
    int current_phase = 0;
    
    void step(int phase);
};
            
