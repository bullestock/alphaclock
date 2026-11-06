#include "hand.h"
#include "hw.h"
#include "stepper.h"
#include "websocket.h"

#include <cmath>

#include "esp_log.h"
#include "esp_system.h"

constexpr const char* TAG = "hand";

Hand h_hours(s_hours);
Hand h_minutes(s_minutes);
Hand h_seconds(s_seconds);

Hand::Hand(Stepper& m)
    : motor(m)
{
}

bool Hand::home()
{
    const auto index = motor.get_index();
    const auto& calibration = motor.get_calibration();
    const auto max_steps = static_cast<int>(calibration.steps * 1.2);
    // First make sure that sensor is not active
    if (is_sensor_activated(index))
    {
        ESP_LOGI(TAG, "ensure inactive");
        motor.start(false, 0);
        bool ok = false;
        int i = 0;
        while (i < 100)
        {
            if ((i % 4) == 0)
                printf("pos %d\n", motor.get_position());
            if (!is_sensor_activated(index))
            {
                ok = true;
                break;
            }
            vTaskDelay(1);
            ++i;
        }
        motor.stop(true);
        if (!ok)
        {
            printf("Failed to clear sensor for motor %d (waited %d)\n", index, i);
            return false;
        }
    }
    // Now do the initial seek: move forward until sensor is activated
    ESP_LOGI(TAG, "seek");
    motor.start(true, 0);
    bool ok = false;
    int i = 0;
    while (1)
    {
        vTaskDelay(1);
        if (is_sensor_activated(index))
        {
            ok = true;
            break;
        }
        ++i;
        if (i > 10000)
            break;
        int step_count = motor.get_step_count();
        if (step_count > max_steps)
            break;
    }
    motor.stop(true);
    if (!ok)
    {
        printf("Failed to locate home position for motor %d\n", index);
        return false;
    }
    // Back off slowly: move backward until sensor is no longer activated
    ESP_LOGI(TAG, "back off");
    motor.start(false, 2);
    ok = false;
    for (int i = 0; i < 100; ++i)
    {
        vTaskDelay(1);
        if (!is_sensor_activated(index))
        {
            ok = true;
            break;
        }
    }
    motor.stop(true);
    if (!ok)
    {
        printf("Failed to back off for motor %d\n", index);
        return false;
    }
    // Final slow home: move forward until sensor is activated
    ESP_LOGI(TAG, "homing");
    motor.start(true, 2);
    ok = false;
    for (int i = 0; i < 100; ++i)
    {
        vTaskDelay(1);
        if (is_sensor_activated(index))
        {
            ok = true;
            break;
        }
    }
    motor.stop(true);
    if (!ok)
    {
        printf("Failed to home motor %d\n", index);
        return false;
    }
    motor.zero();
    return true;
}
    
void Hand::go_to(int position,
                 bool wait)
{
    // Compute new absolute position
    const auto& calibration = motor.get_calibration();
    const int target_motor_position = Stepper::wrap(std::round(calibration.steps * (position - calibration.sensor_position) / 60.0),
                                                    calibration.steps);
    const int current_motor_position = motor.get_position();
    const bool debug_motor = get_motor_debug(motor.get_index());
    
    int steps = target_motor_position - current_motor_position;
    if (!steps)
        return;
    
    if (debug_motor)
    {
        printf("Motor %d: Go to %d (with sensor position %.1f)\n",
               motor.get_index(), position, calibration.sensor_position);
        printf("Moving motor %d (cal %.1f) from %d to %d (%d/60):\n",
               motor.get_index(), calibration.steps,
               current_motor_position, target_motor_position, position);
    }

    bool reverse = steps < 0;
    if (debug_motor)
        printf("diff %d reverse %d\n", steps, reverse);
    if (std::abs(steps) > calibration.steps/2.0)
    {
        // We are more than 180 degrees from the target - or are we?
        const auto wrapped_target_motor_position = target_motor_position + calibration.steps;
        const int wrapped_steps = wrapped_target_motor_position - current_motor_position;
        if (std::abs(wrapped_steps) < std::abs(steps))
        {
            // It is faster to go the other way around
            steps = wrapped_steps;
            if (debug_motor)
                printf("Wrapped: %d steps reverse %d\n", steps, reverse);
        }
        else
        {
            if (debug_motor)
                printf("Forward: %d steps reverse %d\n", steps, reverse);
            steps = std::round(calibration.steps - std::abs(steps));
            reverse = !reverse;
            if (debug_motor)
                printf("Reverse: %d steps reverse %d\n", steps, reverse);
        }
    }
    motor.go_to(target_motor_position, get_motor_delay(), !reverse);
    motor.wait();
    current_position = position;
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
    
void Hand::go_to_hour(int hour, int fraction, bool wait)
{
    if (hour >= 12)
        hour -= 12;
    if (hour >= 12)
    {
        printf("Fatal error: hour %d\n", hour);
        esp_restart();
    }
    int position = hour_map[hour] * 60/12 + fraction/12;
    printf("Moving hour hand to logical %d, physical %d\n", hour, position);
    go_to(position, wait);
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
    if (sec >= 0)
        h_seconds.go_to(sec, false);
    h_minutes.go_to(min, false);
    h_hours.go_to_hour(hour, fraction, false);
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
