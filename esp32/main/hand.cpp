#include "hand.h"
#include "hw.h"
#include "stepper.h"
#include "websocket.h"

#include <cmath>

#include "esp_log.h"
#include "esp_system.h"

constexpr const char* TAG = "hand";

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
    
bool Hand::home()
{
    const auto index = motor.get_index();
    const auto max_steps = static_cast<int>(motor.get_calibration().steps * 1.2);
    // First make sure that sensor is not active
    int i = 0;
    ESP_LOGI(TAG, "ensure inactive");
    motor.start(true, 1);
    while (is_sensor_activated(index) && i < 1000)
    {
        vTaskDelay(1);
        ++i;
    }
    motor.stop(true);
    if (is_sensor_activated(index))
    {
        printf("Failed to clear sensor for motor %d\n", index);
        return false;
    }
    // Now do the initial seek
    ESP_LOGI(TAG, "seek");
    motor.start(motor.get_calibration().reverse, 1);
    while (1)
    {
        vTaskDelay(1);
        if (is_sensor_activated(index))
            break;
        int step_count = motor.get_step_count();
        if (step_count > max_steps)
            break;
    }
    motor.stop(true);
    if (!is_sensor_activated(index))
    {
        printf("Failed to locate home position for motor %d\n", index);
        return false;
    }
    // Back off slowly
    ESP_LOGI(TAG, "back off");
    motor.start(!motor.get_calibration().reverse, 5);
    for (int i = 0; i < 100; ++i)
    {
        vTaskDelay(1);
        if (!is_sensor_activated(index))
            break;
    }
    motor.stop(true);
    if (is_sensor_activated(index))
    {
        printf("Failed to back off for motor %d\n", index);
        return false;
    }
    // Final slow home
    ESP_LOGI(TAG, "homing");
    motor.start(motor.get_calibration().reverse, 5);
    for (int i = 0; i < 100; ++i)
    {
        vTaskDelay(1);
        if (is_sensor_activated(index))
            break;
    }
    motor.stop(true);
    if (!is_sensor_activated(index))
    {
        printf("Failed to home motor %d\n", index);
        return false;
    }
    ESP_LOGI(TAG, "apply offset");
    motor.step(motor.get_calibration().offset *
               motor.get_calibration().steps / 60.0, 1, true);
    return true;
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

Hand& get_hand(int hand)
{
    switch (hand)
    {
    case 0:
        return h_hours;
    case 1:
        return h_minutes;
    default:
        return h_seconds;
    }
}

// Local Variables:
// compile-command: "(cd ..; idf.py build)"
// End:
