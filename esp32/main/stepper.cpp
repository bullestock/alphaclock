#include "stepper.h"
#include "hw.h"

#include <limits>
#include <numbers>
#include <math.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/ledc.h>
#include <esp_timer.h>
#include <rom/gpio.h>

struct Data
{
    bool forward;
    /// Delay in ticks
    int delay;
    /// Remaining ticks
    int remaining_ticks;
    Stepper::State state;
    bool stopping;
    /// I2S bits
    int step_bit;
    int dir_bit;
    int step_count;
    // Relative to homing position
    int current_position;
    int target_position;
    int sensor_activated_at;
};

Data motor_data[MOTOR_COUNT];

/// Instance count
static int count = 0;

Stepper s_hours(5, 6);   // X: QC, QB
Stepper s_minutes(1, 2); // Y: QG, QF
Stepper s_seconds(3, 4); // Z: QE, QD

// Maximum number of steps that sensor can be activated for (i.e. sensor dead zone)
static const int SENSOR_ACTIVATION_COUNT = 2500;

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

    bool stopping_copy[MOTOR_COUNT];
    for (int motor = 0; motor < MOTOR_COUNT; ++motor)
        stopping_copy[motor] = motor_data[motor].stopping;
    
    for (int motor = 0; motor < MOTOR_COUNT; ++motor)
    {
        auto& data = motor_data[motor];
        
        if (stopping_copy[motor])
            data.state = Stepper::State::Idle;
        if (data.state == Stepper::State::Idle)
            continue;
        enabled = true;
        switch (data.state)
        {
        case Stepper::State::CountDown:
            if (data.remaining_ticks > 0)
                --data.remaining_ticks;
            else
            {
                data.remaining_ticks = data.delay;
                // Set DIR pin
                data.state = Stepper::State::DirectionSet;
                if (data.forward)
                    i2s_data |= 1 << data.dir_bit;
            }
            break;
            
        case Stepper::State::DirectionSet:
            // Preserve DIR pin state
            if (data.forward)
                i2s_data |= 1 << data.dir_bit;
            // Set STEP pin
            i2s_data |= 1 << data.step_bit;
            data.state = Stepper::State::StepSet1;
            break;

        case Stepper::State::StepSet1:
            // Keep STEP pin set
            i2s_data |= 1 << data.step_bit;
            data.state = Stepper::State::StepSet2;
            break;
            
        case Stepper::State::StepSet2:
            ++data.step_count;
            data.current_position += data.forward ? 1 : -1;
            // Clear STEP pin
            if (data.target_position != data.current_position)
                data.state = Stepper::State::CountDown;
            else
                data.state = Stepper::State::Idle;
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
    for (int motor = 0; motor < MOTOR_COUNT; ++motor)
        if (stopping_copy[motor])
            motor_data[motor].stopping = false;

    // Check homing sensors
    for (int motor = 0; motor < MOTOR_COUNT; ++motor)
    {
        const bool activated = is_sensor_activated(motor);
        if (motor_data[motor].current_position < SENSOR_ACTIVATION_COUNT)
            ; //sensor_activated_at[motor] = -1;
        else if (activated)
            if (motor_data[motor].sensor_activated_at < 0)
                motor_data[motor].sensor_activated_at = motor_data[motor].current_position;
    }

    return false;
}

static gptimer_handle_t gptimer = nullptr;

Stepper::Stepper(int _dir_bit,
                 int _step_bit)
{
    if (count == 0)
    {
        for (auto& d : motor_data)
        {
            d.state = State::Idle;
            d.stopping = false;
            d.sensor_activated_at = -1;
        }

        // First instance: Set up timer
        gptimer_config_t config = {
            .clk_src = GPTIMER_CLK_SRC_DEFAULT,
            .direction = GPTIMER_COUNT_UP,
            .resolution_hz = 1 * 1000 * 1000, // 1MHz, 1 tick = 1us
            .intr_priority = 0,
            .flags = { },
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
    motor_data[motor].dir_bit = _dir_bit;
    motor_data[motor].step_bit = _step_bit;
}

int Stepper::get_index() const
{
    return motor;
}

const calibration_data& Stepper::get_calibration() const
{
    return ::get_calibration(motor);
}

void Stepper::step(int nof_steps, uint64_t delay_us, bool wait, bool debug)
{
    auto& data = motor_data[motor];
    
    data.state = State::Idle;

    debug |= get_motor_debug(motor);
    
    data.forward = nof_steps > 0;
    data.target_position = data.current_position + nof_steps;
    data.delay = delay_us;
    data.remaining_ticks = 1;
    
    data.state = State::CountDown;

    if (!wait)
        return;

    bool prev_sensor_state[MOTOR_COUNT];
    if (debug)
    {
        for (int motor = 0; motor < MOTOR_COUNT; ++motor)
            prev_sensor_state[motor] = is_sensor_activated(motor);
        for (int motor = 0; motor < MOTOR_COUNT; ++motor)
            printf("%d", prev_sensor_state[motor]);
        printf("\n");
    }
    
    while (1)
    {
        const auto current = data.current_position;
        const auto target = data.target_position;
        if (current == target)
            break;
        //printf("cur %d target %d\n", current, target);
        vTaskDelay(1);
        if (debug)
        {
            bool sensor_state[MOTOR_COUNT];
            int position[MOTOR_COUNT];
            bool changed = false;
            for (int motor = 0; motor < MOTOR_COUNT; ++motor)
            {
                sensor_state[motor] = is_sensor_activated(motor);
                position[motor] = motor_data[motor].current_position;
                if (sensor_state[motor] != prev_sensor_state[motor])
                    changed = true;
            }
            if (changed)
            {
                for (int motor = 0; motor < MOTOR_COUNT; ++motor)
                    printf("%d", sensor_state[motor]);
                printf("    ");
                for (int motor = 0; motor < MOTOR_COUNT; ++motor)
                    printf("%d ", position[motor]);
                printf("\n");
                for (int motor = 0; motor < MOTOR_COUNT; ++motor)
                    prev_sensor_state[motor] = sensor_state[motor];
            }
        }
    }
}

void Stepper::start(bool forward, uint64_t delay_us)
{
    auto& data = motor_data[motor];

    data.state = State::Idle;
    data.step_count = 0;

    data.forward = forward;
    data.target_position = data.current_position + forward ? -1 : 1;
    data.delay = delay_us;
    data.remaining_ticks = 1;

    data.stopping = false;
    data.state = State::CountDown;
}

void Stepper::stop(bool wait)
{
    motor_data[motor].stopping = true;
    if (!wait)
        return;
    while (motor_data[motor].stopping)
        vTaskDelay(1);
}

void Stepper::wait()
{
    while (motor_data[motor].state != State::Idle)
        vTaskDelay(10 / portTICK_PERIOD_MS);
}

int Stepper::get_step_count() const
{
    return motor_data[motor].step_count;
}

int Stepper::get_position() const
{
    return motor_data[motor].current_position;
}

void Stepper::zero()
{
    auto& data = motor_data[motor];
    data.current_position = 0;
    data.target_position = 0;
}

int Stepper::get_sensor_activated_at() const
{
    return motor_data[motor].sensor_activated_at;
}

void Stepper::clear_sensor_activated_at()
{
    motor_data[motor].sensor_activated_at = -1;
}

void Stepper::go_to(int position)
{
    motor_data[motor].target_position = position;
}

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:
