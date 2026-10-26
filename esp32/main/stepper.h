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

    /// Get index (H/M/S)
    int get_index() const;

    const calibration_data& get_calibration() const;

    void step(int nof_steps, uint64_t delay_us,
              bool wait = false, bool debug = false);

    void start(bool forward, uint64_t delay_us);

    void stop(bool wait = false);

    // Wait for current operation to complete
    void wait();

    // Return number of steps performed since last call to start()
    int get_step_count() const;

    /// Set current position as absolute zero
    void zero();

    /// Get current absolute position.
    int get_position() const;

    /// Get absolute position at which the homing sensor was activated.
    int get_sensor_activated_at() const;

    void clear_sensor_activated_at();

    /// Go to the specified absolute position.
    void go_to(int position);

private:
    int motor = 0;
    int steps = 0;
};

extern Stepper s_hours, s_minutes, s_seconds;
