#include "stepper.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/gptimer.h>
#include <esp_timer.h>
#include <rom/gpio.h>

static bool step_enable = false;
static bool step_forward = false;
static int steps_left = 0;
static int current_phase = 0;

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

static bool timer_isr_callback(gptimer_handle_t, const gptimer_alarm_event_data_t*, void*)
{
    if (!step_enable)
        return false;

    if (steps_left > 0)
    {
        --steps_left;
        REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << PIN_EN);

        if (step_forward)
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
        step(current_phase);
        return false;
    }
    // no more work, disable driver
    REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << PIN_EN);
    step_enable = false;
    return false;
}

static gptimer_handle_t gptimer = nullptr;

Stepper::Stepper()
{
    gptimer_config_t config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1 * 1000 * 1000, // 1MHz, 1 tick = 1us
        .intr_priority = 0,
        //.intr_shared = 0,
        //.backup_before_sleep = 0,
        .flags = 0,
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&config, &gptimer));
}

void Stepper::step(int nof_steps, uint64_t delay_us)
{
    if (timer_enabled)
    {
        gptimer_stop(gptimer);
        gptimer_disable(gptimer);
    }
    
    step_enable = false;
    step_forward = nof_steps > 0;
    steps_left = abs(nof_steps);

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

    step_enable = true;
}

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:
