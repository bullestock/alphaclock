#include "stepper.h"

#include <numbers>
#include <math.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/gptimer.h>
#include <driver/ledc.h>
#include <esp_timer.h>
#include <rom/gpio.h>

/// True if this motor is active; if not, all other variables are ignored
static bool step_enable[MOTOR_COUNT];
/// True if this motor goes forward
static bool step_forward[MOTOR_COUNT];
/// How many steps are left
static int steps_left[MOTOR_COUNT];
/// Delay in ticks for each motor
static int delay[MOTOR_COUNT];
/// Remaining ticks for each motor
static int remaining_ticks[MOTOR_COUNT];
/// Is the STEP pin active?
static bool step_pin_active[MOTOR_COUNT];
/// I2S bits
static int step_bit[MOTOR_COUNT];
static int dir_bit[MOTOR_COUNT];
/// Instance count
static int count = 0;

Stepper s_hours(2, 1); // X
Stepper s_minutes(6, 5); // Y
Stepper s_seconds(4, 3); // Z


void IRAM_ATTR i2s_shiftout(uint8_t data)
{
    // WS = 0
    REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << 12);
    int mask = 1;
    for (int i = 0; i < 8; ++i)
    {
        // Write a data bit to DATA
        if (mask & data)
            REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << 14);
        else
            REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << 14);
        // BCK = 1
        REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << 11);
        // BCK = 0
        REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << 11);
        mask <<= 1;
    }
    // WS = 1: Latch
    REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << 12);
}

static bool IRAM_ATTR timer_isr_callback(gptimer_handle_t,
                                         const gptimer_alarm_event_data_t*,
                                         void*)
{
    static uint8_t last_i2s_data = 0;
    uint8_t i2s_data = 0;
    bool enabled = false;
    for (int motor = 0; motor < MOTOR_COUNT; ++motor)
    {
        if (!step_enable[motor])
            continue;
        enabled = true;
        if (step_pin_active[motor])
        {
            // Turn step pin off
            //i2s_data &= ~(1 << step_bit[motor]);
            step_pin_active[motor] = false;
            // Prepare for next step
            remaining_ticks[motor] = delay[motor];
            continue;
        }
        if (remaining_ticks[motor] > 0)
        {
            --remaining_ticks[motor];
            continue;
        }
        if (steps_left[motor] > 0)
        {
            --steps_left[motor];
            // Time expired, set direction pin
            if (step_forward[motor])
                i2s_data |= 1 << dir_bit[motor];
            //else
            //    i2s_data &= ~(1 << dir_bit[motor]);
            // Turn step pin on
            i2s_data |= 1 << step_bit[motor];
            step_pin_active[motor] = true;
        }
        else
            step_enable[motor] = false;
    }
    if (enabled)
        i2s_data |= 1;
    if (i2s_data != last_i2s_data)
    {
        i2s_shiftout(i2s_data);
        last_i2s_data = i2s_data;
    }
    return false;
}

static gptimer_handle_t gptimer = nullptr;

Stepper::Stepper(int _dir_bit,
                 int _step_bit)
{
    if (count == 0)
    {
        // First instance: Set up timer
        gptimer_config_t config = {
            .clk_src = GPTIMER_CLK_SRC_DEFAULT,
            .direction = GPTIMER_COUNT_UP,
            .resolution_hz = 1 * 1000 * 1000, // 1MHz, 1 tick = 1us
            .intr_priority = 0,
            .flags = {
                .intr_shared = 0,
                .allow_pd = 0,
                .backup_before_sleep = 0,
            },
        };
        ESP_ERROR_CHECK(gptimer_new_timer(&config, &gptimer));
        gptimer_event_callbacks_t cbs = {
            .on_alarm = timer_isr_callback,
        };
        ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, nullptr));
        gptimer_alarm_config_t alarm_config = {
            .alarm_count = 100,
            .reload_count = 0, // counter will reload with 0 on alarm event
            .flags = 0,
        };
        alarm_config.flags.auto_reload_on_alarm = true;
        ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
        ESP_ERROR_CHECK(gptimer_enable(gptimer));
        ESP_ERROR_CHECK(gptimer_start(gptimer));
    }
    motor = count;
    dir_bit[count] = _dir_bit;
    step_bit[count++] = _step_bit;
}

int Stepper::get_index() const
{
    return motor;
}

const calibration_data& Stepper::get_calibration()
{
    return ::get_calibration(motor);
}

void Stepper::step(int nof_steps, uint64_t delay_us, bool wait)
{
    step_enable[motor] = false;

    // TODO
    
    step_enable[motor] = true;

    if (!wait)
        return;

    while (steps_left[motor] > 0)
        vTaskDelay(1);
}

void Stepper::start(bool forward, uint64_t delay_us)
{
    step_forward[motor] = forward;

    // TODO
    //steps_left[motor] = 10000;

    step_enable[motor] = true;
}

void Stepper::stop()
{
    step_enable[motor] = false;
}

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:
