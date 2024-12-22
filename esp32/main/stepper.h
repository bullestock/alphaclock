#pragma once

#include "defs.h"

class Stepper
{
public:
    Stepper(gpio_num_t a1,
            gpio_num_t a2,
            gpio_num_t b1,
            gpio_num_t b2);

    void step(int nof_steps, int64_t delay_us);

private:
    gpio_num_t pin_a1 = (gpio_num_t) 0;
    gpio_num_t pin_a2 = (gpio_num_t) 0;
    gpio_num_t pin_b1 = (gpio_num_t) 0;
    gpio_num_t pin_b2 = (gpio_num_t) 0;
    int current_phase = 0;
    
    void step(int phase);
};
            
