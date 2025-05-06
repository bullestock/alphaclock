#include "stepper.h"

#include <numbers>
#include <math.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/gptimer.h>
#include <driver/ledc.h>
#include <esp_timer.h>
#include <rom/gpio.h>

const int NOF_MOTORS = 3;

const int MAX_PWM = 1024; // must match timer resolution

const bool USE_MICRO_STEPPING = true;
// Must be a multiple of 4
const int NOF_MICRO_STEPS = USE_MICRO_STEPPING ? 16 : 4;

static bool step_enable[NOF_MOTORS];
static bool step_forward[NOF_MOTORS];
static int steps_left[NOF_MOTORS];
static int current_phase[NOF_MOTORS];
static int enable_pin[NOF_MOTORS];
static int count = 0;
static bool timer_enabled = false;

int calc_pwm(int phase, int offset)
{
    return int(cos(((phase + 0) +
                    (offset/4.0 * NOF_MICRO_STEPS))/
                   NOF_MICRO_STEPS * 2.0 * std::numbers::pi) * static_cast<float>(MAX_PWM));
}

// Note that this function only works if A/B pins are below GPIO32
static void step(int phase)
{
    if (USE_MICRO_STEPPING)
    {
        // each coil has sinusoidal pattern offset 90 degrees from previous coil
        int coil1a_pwm = calc_pwm(phase, 0);
        int coil1b_pwm = calc_pwm(phase, 2);
        int coil2a_pwm = calc_pwm(phase, 3);
        int coil2b_pwm = calc_pwm(phase, 1);
    
        // half rectify each wave
        if (coil1a_pwm < 0)
            coil1a_pwm = 0;
        if (coil1b_pwm < 0)
            coil1b_pwm = 0;
        if (coil2a_pwm < 0)
            coil2a_pwm = 0;
        if (coil2b_pwm < 0)
            coil2b_pwm = 0;

        //printf("%3d %3d %3d %3d\n", coil1a_pwm, coil1b_pwm, coil2a_pwm, coil2b_pwm);
        
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, coil1a_pwm));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, coil1b_pwm));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1));
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, coil2a_pwm));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2));
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3, coil2b_pwm));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3));
    }
    else
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
}

static bool timer_isr_callback(gptimer_handle_t,
                               const gptimer_alarm_event_data_t*,
                               void*)
{
    for (int motor = 0; motor < NOF_MOTORS; ++motor)
    {
        int enable = enable_pin[motor];
        const bool bank1 = enable >= 32;
        if (bank1)
            enable -= 32;
        if (!step_enable[motor])
        {
            if (bank1)
                REG_WRITE(GPIO_OUT1_W1TC_REG, 1ULL << enable);
            else
                REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << enable);
            continue;
        }

        if (steps_left[motor] > 0)
        {
            --steps_left[motor];
            if (bank1)
                REG_WRITE(GPIO_OUT1_W1TS_REG, 1ULL << enable);
            else
                REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << enable);

            if (step_forward[motor])
            {
                ++current_phase[motor];
                if (current_phase[motor] >= NOF_MICRO_STEPS)
                    current_phase[motor] = 0;
            }
            else
            {
                --current_phase[motor];
                if (current_phase[motor] < 0)
                    current_phase[motor] = NOF_MICRO_STEPS - 1;
            }
            step(current_phase[motor]);
        }
        else
        {
            // no more work, disable driver
            if (bank1)
                REG_WRITE(GPIO_OUT1_W1TC_REG, 1ULL << enable);
            else
                REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << enable);
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
            .flags = {
                .intr_shared = 0,
                .allow_pd = 0,
                .backup_before_sleep = 0,
            },
        };
        ESP_ERROR_CHECK(gptimer_new_timer(&config, &gptimer));

        ledc_timer_config_t ledc_timer = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = LEDC_TIMER_10_BIT,
            .timer_num = LEDC_TIMER_0,
            .freq_hz = 20000,
            .clk_cfg = LEDC_AUTO_CLK,
            .deconfigure = 0,
        };
        ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

        ledc_channel_config_t pwm_channel = {
            .gpio_num = PIN_A1,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = LEDC_CHANNEL_0,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = LEDC_TIMER_0,
            .duty = 0,
            .hpoint = 0,
            .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
            .flags = 0,
        };
        ESP_ERROR_CHECK(ledc_channel_config(&pwm_channel));

        pwm_channel.gpio_num = PIN_A2;
        pwm_channel.channel = LEDC_CHANNEL_1;
        ESP_ERROR_CHECK(ledc_channel_config(&pwm_channel));

        pwm_channel.gpio_num = PIN_B1;
        pwm_channel.channel = LEDC_CHANNEL_2;
        ESP_ERROR_CHECK(ledc_channel_config(&pwm_channel));

        pwm_channel.gpio_num = PIN_B2;
        pwm_channel.channel = LEDC_CHANNEL_3;
        ESP_ERROR_CHECK(ledc_channel_config(&pwm_channel));
    }
    motor = count;
    enable_pin[count++] = _enable_pin;
}

void enable_timer()
{
    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_isr_callback,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, nullptr));
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    timer_enabled = true;
    ESP_ERROR_CHECK(gptimer_start(gptimer));
}

void set_timer_rate(uint64_t delay_us)
{
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = delay_us,
        .reload_count = 0, // counter will reload with 0 on alarm event
        .flags = 0,
    };
    alarm_config.flags.auto_reload_on_alarm = true;
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
}

void Stepper::step(int nof_steps, uint64_t delay_us, bool wait)
{
    nof_steps *= NOF_MICRO_STEPS;
    
    step_enable[motor] = false;
    step_forward[motor] = nof_steps > 0;
    steps_left[motor] = abs(nof_steps);

    set_timer_rate(delay_us);

    if (!timer_enabled)
        enable_timer();

    step_enable[motor] = true;

    if (!wait)
        return;

    while (steps_left[motor] > 0)
        vTaskDelay(1);
}

void Stepper::start(bool forward, uint64_t delay_us)
{
    step_forward[motor] = forward;
    steps_left[motor] = 10000;

    set_timer_rate(delay_us);

    if (!timer_enabled)
        enable_timer();

    step_enable[motor] = true;
}

void Stepper::stop()
{
    step_enable[motor] = false;
}

bool Stepper::busy() const
{
    return step_enable[motor];
}

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:
