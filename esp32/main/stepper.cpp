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
const int NOF_MICRO_STEPS = 4;

const bool USE_MICRO_STEPPING = true;

static bool step_enable[NOF_MOTORS];
static bool step_forward[NOF_MOTORS];
static int steps_left[NOF_MOTORS];
static int current_phase[NOF_MOTORS];
static int enable_pin[NOF_MOTORS];
static int count = 0;
static bool timer_enabled = false;

// Note that this function only works if A/B pins are below GPIO32
static void step(int phase)
{
    if (USE_MICRO_STEPPING)
    {
        // each coil has sinusoidal pattern offset pi/2 rad from previous coil
        int coil1a_pwm = int(cos(((phase + 1)+(0.0/4.0*NOF_MICRO_STEPS))/NOF_MICRO_STEPS*2.0*std::numbers::pi)*255.0);
        int coil1b_pwm = int(cos(((phase + 1)+(2.0/4.0*NOF_MICRO_STEPS))/NOF_MICRO_STEPS*2.0*std::numbers::pi)*255.0);
        int coil2a_pwm = int(cos(((phase + 1)+(3.0/4.0*NOF_MICRO_STEPS))/NOF_MICRO_STEPS*2.0*std::numbers::pi)*255.0);
        int coil2b_pwm = int(cos(((phase + 1)+(1.0/4.0*NOF_MICRO_STEPS))/NOF_MICRO_STEPS*2.0*std::numbers::pi)*255.0);
    
        // half rectify each wave
        if (coil1a_pwm < 0) {
            coil1a_pwm = 0;
        }
        if (coil1b_pwm < 0) {
            coil1b_pwm = 0;
        }
        if (coil2a_pwm < 0) {
            coil2a_pwm = 0;
        }
        if (coil2b_pwm < 0) {
            coil2b_pwm = 0;
        }

        switch (phase) {
        case 0:  // 1010
            printf("1010\n");
            coil1a_pwm = coil2a_pwm = 255;
            coil1b_pwm = coil2b_pwm = 0;
            break;
        case 1:  // 0110
            printf("0110\n");
            coil1b_pwm = coil2a_pwm = 255;
            coil1a_pwm = coil2b_pwm = 0;
            break;
        case 2:  //0101
            printf("0101\n");
            coil1b_pwm = coil2b_pwm = 255;
            coil1a_pwm = coil2a_pwm = 0;
            break;
        case 3:  //1001
            printf("1001\n");
            coil1a_pwm = coil2b_pwm = 255;
            coil1b_pwm = coil2a_pwm = 0;
            break;
        }
        printf("%3d %3d %3d %3d\n",
               coil1a_pwm, coil1b_pwm, coil2a_pwm, coil2b_pwm);
        
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
            printf("1010\n");
            REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << PIN_A1);
            REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << PIN_A2);
            REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << PIN_B1);
            REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << PIN_B2);
            break;
        case 1:  // 0110
            printf("0110\n");
            REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << PIN_A1);
            REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << PIN_A2);
            REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << PIN_B1);
            REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << PIN_B2);
            break;
        case 2:  //0101
            printf("0101\n");
            REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << PIN_A1);
            REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << PIN_A2);
            REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << PIN_B1);
            REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << PIN_B2);
            break;
        case 3:  //1001
            printf("1001\n");
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
        if (!step_enable[motor])
            continue;

        int enable = enable_pin[motor];
        const bool bank1 = enable >= 32;
        if (bank1)
            enable -= 32;
        
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
            .duty_resolution = LEDC_TIMER_8_BIT,
            .timer_num = LEDC_TIMER_0,
            .freq_hz = 200, // chairbot
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

void Stepper::step(int nof_steps, uint64_t delay_us)
{
#if 0
    // interrupt driven
    
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
#else
    // busy wait

    int enable = enable_pin[motor];
    const bool bank1 = enable >= 32;
    if (bank1)
        enable -= 32;
    if (bank1)
        REG_WRITE(GPIO_OUT1_W1TS_REG, 1ULL << enable);
    else
        REG_WRITE(GPIO_OUT_W1TS_REG, 1ULL << enable);

    int abs_nof_steps = abs(nof_steps);
    int current_phase = 0;
    while (abs_nof_steps > 0)
    {
        ::step(current_phase);
        if (nof_steps > 0)
        {
            ++current_phase;
            if (current_phase >= NOF_MICRO_STEPS)
                current_phase = 0;
        }
        else
        {
            --current_phase;
            if (current_phase < 0)
                current_phase = NOF_MICRO_STEPS - 1;
        }
        vTaskDelay(delay_us / 1000 / portTICK_PERIOD_MS);
        --abs_nof_steps;
    }

    if (bank1)
        REG_WRITE(GPIO_OUT1_W1TC_REG, 1ULL << enable);
    else
        REG_WRITE(GPIO_OUT_W1TC_REG, 1ULL << enable);
#endif
}

bool Stepper::busy() const
{
    return step_enable[motor];
}

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:
