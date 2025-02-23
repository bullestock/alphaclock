#include "stepper.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/gptimer.h>
#include <esp_timer.h>
#include <rom/gpio.h>

const int NOF_MOTORS = 3;

static bool step_enable[NOF_MOTORS];
static bool step_forward[NOF_MOTORS];
static int steps_left[NOF_MOTORS];
static int current_phase[NOF_MOTORS];
static int enable_pin[NOF_MOTORS];
static int count = 0;
static bool timer_enabled = false;

static void step(int phase)
{
    switch (phase) {
    case 0:  // 1010
        REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << PIN_A1);
        REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << PIN_A2);
        REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << PIN_B1);
        REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << PIN_B2);
        break;
    case 1:  // 0110
        REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << PIN_A1);
        REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << PIN_A2);
        REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << PIN_B1);
        REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << PIN_B2);
        break;
    case 2:  //0101
        REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << PIN_A1);
        REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << PIN_A2);
        REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << PIN_B1);
        REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << PIN_B2);
        break;
    case 3:  //1001
        REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << PIN_A1);
        REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << PIN_A2);
        REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << PIN_B1);
        REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << PIN_B2);
        break;
    }
}

static bool timer_isr_callback(gptimer_handle_t,
                               const gptimer_alarm_event_data_t*,
                               void*)
{
    for (int motor = 0; motor < NOF_MOTORS; ++motor)
    {
        if (!step_enable[motor])
            continue;

        if (steps_left[motor] > 0)
        {
            --steps_left[motor];
            REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << enable_pin[motor]);

            if (step_forward[motor])
            {
                ++current_phase[motor];
                if (current_phase[motor] == 4)
                    current_phase[motor] = 0;
            }
            else
            {
                if (current_phase[motor] == 0)
                    current_phase[motor] = 4;
                --current_phase[motor];
            }
            step(current_phase[motor]);
        }
        else
        {
            // no more work, disable driver
            REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << enable_pin[motor]);
            step_enable[motor] = false;
        }
    }
    return false;
}

static gptimer_handle_t gptimer = nullptr;

Stepper::Stepper(int _enable_pin)
{
    if (count == 0)
    {
        gptimer_config_t config = {
            .clk_src = GPTIMER_CLK_SRC_DEFAULT,
            .direction = GPTIMER_COUNT_UP,
            .resolution_hz = 1 * 1000 * 1000, // 1MHz, 1 tick = 1us
            .intr_priority = 0,
            .flags = 0,
        };
        ESP_ERROR_CHECK(gptimer_new_timer(&config, &gptimer));
    }
    motor = count;
    enable_pin[count++] = _enable_pin;
}

void Stepper::step(int nof_steps, uint64_t delay_us)
{
    if (timer_enabled)
    {
        gptimer_stop(gptimer);
        gptimer_disable(gptimer);
    }
    
    step_enable[motor] = false;
    step_forward[motor] = nof_steps > 0;
    steps_left[motor] = abs(nof_steps);

    gptimer_alarm_config_t alarm_config = {
        .alarm_count = delay_us,
        .reload_count = 0, // counter will reload with 0 on alarm event
        .flags = 0,
    };
    alarm_config.flags.auto_reload_on_alarm = true;
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_isr_callback,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, nullptr));
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    timer_enabled = true;
    ESP_ERROR_CHECK(gptimer_start(gptimer));

    step_enable[motor] = true;
}

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:
