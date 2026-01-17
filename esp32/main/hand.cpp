#include "hand.h"
#include "stepper.h"
#include "websocket.h"

#include <cmath>

#include "esp_system.h"

static bool debug_motor = false;

Hand h_hours(s_hours);
Hand h_minutes(s_minutes);
Hand h_seconds(s_seconds);

Hand::Hand(Stepper& m)
    : motor(m)
{
}

void Hand::set_debug(bool on)
{
    debug_motor = on;
}

void Hand::zero()
{
    current_position = 0;
}
    
void Hand::go_to(int position,
                 bool wait)
{
    // Compute new absolute position
    const auto& calibration = motor.get_calibration();
    const int target_steps = std::round(calibration.steps * position / 60.0);

    if (debug_motor)
    {
        printf("Moving motor %d (cal %.1f) from %d to %d (%d/60):\n",
               motor.get_index(), calibration.steps,
               current_position, target_steps, position);
    }

    int diff_steps = target_steps - current_position;
    bool reverse = calibration.reverse;
    if (debug_motor)
        printf("diff %d reverse %d\n", diff_steps, reverse);
    if (std::abs(diff_steps) > calibration.steps/2.0)
    {
        // We are more than 180 degrees from the target - or are we?
        const auto wrapped_target_steps = target_steps + calibration.steps;
        const int wrapped_diff_steps = wrapped_target_steps - current_position;
        if (std::abs(wrapped_diff_steps) < std::abs(diff_steps))
        {
            // It is faster to go the other way around
            diff_steps = wrapped_diff_steps;
            if (debug_motor)
                printf("Wrapped: %d steps reverse %d\n", diff_steps, reverse);
        }
        else
        {
            if (debug_motor)
                printf("Forward: %d steps reverse %d\n", diff_steps, reverse);
            diff_steps = std::round(calibration.steps - std::abs(diff_steps));
            reverse = !reverse;
            if (debug_motor)
                printf("Reverse: %d steps reverse %d\n", diff_steps, reverse);
        }
    }
    const int steps = (reverse ? -1 : 1) * diff_steps;
    if (debug_motor)
        printf("%d steps\n", steps);
    if (std::abs(steps) > 0)
    {
        motor.step(steps, get_motor_delay(), wait);

        current_position = target_steps;
    }
}

void Hand::wait()
{
    motor.wait();
}

// Hour hand mapping for Danish
static const int hour_map[12] = {
    10, // 0/12
    1,  // 1
    9,  // 2
    11, // 3
    3,  // 4
    2,  // 5
    6, // 6
    7, // 7
    5, // 8
    4, // 9
    8, // 10
    0, // 11
};
    
void Hand::go_to_hour(int hour, int fraction)
{
    if (hour >= 12)
        hour -= 12;
    if (hour >= 12)
    {
        printf("Fatal error: hour %d\n", hour);
        esp_restart();
    }
    int position = hour_map[hour] * 60/12 + fraction/12;
    go_to(position);
}

void set_hands(int hour, int min, int sec)
{
    static int last_hour = -1;

    if (last_hour >= 0)
        if (hour != last_hour)
        {
        }

    int fraction = 0;
    if (active_hour_mode == HOUR_MODE_CONTINUOUS)
        fraction = min;
    h_seconds.go_to(sec, false);
    h_minutes.go_to(min, false);
    h_hours.go_to_hour(hour, fraction);
    h_seconds.wait();
    h_minutes.wait();

    last_hour = hour;
}

// Local Variables:
// compile-command: "(cd ..; idf.py build)"
// End:
