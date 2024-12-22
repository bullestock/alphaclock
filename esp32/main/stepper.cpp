#include "stepper.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_timer.h>

Stepper::Stepper(gpio_num_t a1,
                 gpio_num_t a2,
                 gpio_num_t b1,
                 gpio_num_t b2)
    : pin_a1(a1),
      pin_a2(a2),
      pin_b1(b1),
      pin_b2(b2)
{
}

void Stepper::step(int nof_steps, int64_t delay_us)
{
    int steps_left = abs(nof_steps);

    const bool is_forward = nof_steps > 0;

    int64_t last_step_time = 0;

    while (steps_left > 0)
    {
        const auto now = esp_timer_get_time();
        if (now - last_step_time >= delay_us)
        {
            last_step_time = now;
            if (is_forward)
            {
                ++current_phase;
                if (current_phase == 4)
                    current_phase = 0;
            }
            else
            {
                if (current_phase == 0)
                    current_phase = 4;
                --current_phase;
            }
            --steps_left;
            step(current_phase);
        }
        vTaskDelay(1);
    }
}

void Stepper::step(int phase)
{
    switch (phase) {
    case 0:  // 1010
        ESP_ERROR_CHECK(gpio_set_level(pin_a1, 1));
        ESP_ERROR_CHECK(gpio_set_level(pin_a2, 0));
        ESP_ERROR_CHECK(gpio_set_level(pin_b1, 1));
        ESP_ERROR_CHECK(gpio_set_level(pin_b2, 0));
        break;
    case 1:  // 0110
        ESP_ERROR_CHECK(gpio_set_level(pin_a1, 0));
        ESP_ERROR_CHECK(gpio_set_level(pin_a2, 1));
        ESP_ERROR_CHECK(gpio_set_level(pin_b1, 1));
        ESP_ERROR_CHECK(gpio_set_level(pin_b2, 0));
        break;
    case 2:  //0101
        ESP_ERROR_CHECK(gpio_set_level(pin_a1, 0));
        ESP_ERROR_CHECK(gpio_set_level(pin_a2, 1));
        ESP_ERROR_CHECK(gpio_set_level(pin_b1, 0));
        ESP_ERROR_CHECK(gpio_set_level(pin_b2, 1));
        break;
    case 3:  //1001
        ESP_ERROR_CHECK(gpio_set_level(pin_a1, 1));
        ESP_ERROR_CHECK(gpio_set_level(pin_a2, 0));
        ESP_ERROR_CHECK(gpio_set_level(pin_b1, 0));
        ESP_ERROR_CHECK(gpio_set_level(pin_b2, 1));
        break;
    }
}

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:
