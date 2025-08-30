#include "hand.h"
#include "stepper.h"

#include <cmath>

#include "esp_system.h"

Hand::Hand(Stepper& m)
    : motor(m)
{
}

void Hand::zero()
{
    current_position = 0;
}
    
void Hand::go_to(int position)
{
    // Compute new absolute position
    const auto& calibration = motor.get_calibration();
    const int target_steps = std::round(calibration.steps * position / 60.0);

    printf("Moving motor %d (cal %.1f) from %d to %d (%d/60):\n",
           motor.get_index(), calibration.steps,
           current_position, target_steps, position);

    const int delay = 5000;

    int diff_steps = target_steps - current_position;
    bool reverse = calibration.reverse;
    printf("diff %d reverse %d\n", diff_steps, reverse);
    if (std::abs(diff_steps) > calibration.steps/2.0)
    {
        printf("Forward: %d steps reverse %d\n", diff_steps, reverse);
        diff_steps = std::round(calibration.steps - std::abs(diff_steps));
        reverse = !reverse;
        printf("Reverse: %d steps reverse %d\n", diff_steps, reverse);
    }
    const int steps = (reverse ? -1 : 1) * diff_steps;
    printf("%d steps\n", steps);
    if (std::abs(steps) > 0)
    {
        motor.step(steps, delay, true);

        current_position = target_steps;
    }
}

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

// Local Variables:
// compile-command: "(cd ..; idf.py build)"
// End:
