#pragma once

#include "defs.h"
#include "nvs.h"

#include <driver/gptimer.h>

class Stepper
{
public:
    Stepper(int dir_bit, int step_bit);

    enum class State
    {
        // Idle
        Idle,
        // Waiting for remaining steps to equal zero
        CountDown,
        // We have set the DIR input and can set STEP high on next iteration
        DirectionSet,
        // We have just set STEP high
        StepSet1,
        // We set STEP high in last iteration
        StepSet2,
    };

    int get_index() const;

    const calibration_data& get_calibration();

    void step(int nof_steps, uint64_t delay_us, bool wait = false);

    void start(bool forward, uint64_t delay_us);

    void stop();

    // Wait for current operation to complete
    void wait();

private:
    int motor = 0;
};

extern Stepper s_hours, s_minutes, s_seconds;
