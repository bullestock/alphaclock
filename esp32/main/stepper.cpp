#include "stepper.h"

#include <numbers>
#include <math.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/ledc.h>
#include <esp_timer.h>
#include <rom/gpio.h>

/// True if this motor goes forward
static bool step_forward[MOTOR_COUNT];
/// How many steps are left
static int steps_left[MOTOR_COUNT];
/// Delay in ticks for each motor
static int delay[MOTOR_COUNT];
/// Remaining ticks for each motor
static int remaining_ticks[MOTOR_COUNT];
Stepper::State state[MOTOR_COUNT];
/// I2S bits
static int step_bit[MOTOR_COUNT];
static int dir_bit[MOTOR_COUNT];
/// Instance count
static int count = 0;

Stepper s_hours(5, 6);   // X: QC, QB
Stepper s_minutes(1, 2); // Y: QG, QF
Stepper s_seconds(3, 4); // Z: QE, QD

void IRAM_ATTR i2s_shiftout(uint8_t data)
{
    // WS = 0
    REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << int(PIN_I2S_WS));
    int mask = 1;
    for (int i = 0; i < 8; ++i)
    {
        // Write a data bit to DATA
        if (mask & data)
            REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << int(PIN_I2S_DATA));
        else
            REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << int(PIN_I2S_DATA));
        // BCK = 1
        REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << int(PIN_I2S_BCK));
        // BCK = 0
        REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << int(PIN_I2S_BCK));
        mask <<= 1;
    }
    // WS = 1: Latch
    REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << int(PIN_I2S_WS));
}

#ifdef DEBUG_I2S
uint8_t i2s_debug_data[100];
uint64_t i2s_debug_ts[100];
int i2s_debug_index = 0;
uint64_t i2s_current_debug_ts = 0;
#endif

static bool IRAM_ATTR timer_isr_callback(gptimer_handle_t,
                                         const gptimer_alarm_event_data_t*,
                                         void*)
{
#ifdef DEBUG_I2S
    ++i2s_current_debug_ts;
#endif
    static uint8_t last_i2s_data = 0;
    uint8_t i2s_data = 0;
    bool enabled = false;
    for (int motor = 0; motor < MOTOR_COUNT; ++motor)
    {
        if (state[motor] == Stepper::State::Idle)
            continue;
        enabled = true;
        switch (state[motor])
        {
        case Stepper::State::CountDown:
            if (remaining_ticks[motor] > 0)
                --remaining_ticks[motor];
            else
            {
                remaining_ticks[motor] = delay[motor];
                // Set DIR pin
                state[motor] = Stepper::State::DirectionSet;
                if (step_forward[motor])
                    i2s_data |= 1 << dir_bit[motor];
            }
            break;
            
        case Stepper::State::DirectionSet:
            // Preserve DIR pin state
            if (step_forward[motor])
                i2s_data |= 1 << dir_bit[motor];
            // Set STEP pin
            i2s_data |= 1 << step_bit[motor];
            state[motor] = Stepper::State::StepSet1;
            break;

        case Stepper::State::StepSet1:
            // Keep STEP pin set
            i2s_data |= 1 << step_bit[motor];
            state[motor] = Stepper::State::StepSet2;
            break;
            
        case Stepper::State::StepSet2:
            // Clear STEP pin
            if (steps_left[motor]-- > 0)
                state[motor] = Stepper::State::CountDown;
            else
                state[motor] = Stepper::State::Idle;
            break;

        case Stepper::State::Idle:
            break;

        default:
            assert(false);
            break;
        }
    }
    if (!enabled)
        i2s_data |= 0x80;   // XYZ_EN: QA
    if (i2s_data != last_i2s_data)
    {
#ifdef DEBUG_I2S
        if (i2s_debug_index < sizeof(i2s_debug_data))
        {
            i2s_debug_ts[i2s_debug_index] = i2s_current_debug_ts;
            i2s_debug_data[i2s_debug_index++] = i2s_data;
        }
#endif
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
        for (auto& s : state)
            s = State::Idle;

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
    motor = count++;
    dir_bit[motor] = _dir_bit;
    step_bit[motor] = _step_bit;
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
    state[motor] = State::Idle;

    step_forward[motor] = nof_steps > 0;
    steps_left[motor] = abs(nof_steps);
    delay[motor] = delay_us;
    remaining_ticks[motor] = 1;
    
    state[motor] = State::CountDown;

    if (!wait)
        return;

    while (steps_left[motor] > 0)
        vTaskDelay(1);
}

void Stepper::start(bool forward, uint64_t delay_us)
{
    state[motor] = State::Idle;

    step_forward[motor] = forward;
    steps_left[motor] = 10000;
    delay[motor] = delay_us;
    remaining_ticks[motor] = 1;

    state[motor] = State::CountDown;
}

void Stepper::stop()
{
    state[motor] = State::Idle;
}

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:
