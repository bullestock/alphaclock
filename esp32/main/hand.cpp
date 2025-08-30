#include "hand.h"
#include "stepper.h"

#include <cmath>

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
    if (diff_steps < 0)
        reverse = !reverse;
    printf("diff %d\n", diff_steps);
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

// Local Variables:
// compile-command: "(cd ..; idf.py build)"
// End:
