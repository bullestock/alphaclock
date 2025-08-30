#include "connect.h"
#include "console.h"
#include "defs.h"
#include "hand.h"
#include "hw.h"
#include "nvs.h"
#include "sntp.h"
#include "stepper.h"
#include "websocket.h"

#include <stdio.h>
#include <time.h>

#include <esp_app_desc.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <esp_wifi.h>

void handle_normal_mode();
void handle_fast_mode();

Stepper s_hours(PIN_EN1), s_minutes(PIN_EN2), s_seconds(PIN_EN3);
Hand h_hours(s_hours);
Hand h_minutes(s_minutes);
Hand h_seconds(s_seconds);

extern int active_button;
extern bool button_direction_up;
extern bool is_button_fast;
extern Mode active_mode;
extern HourMode active_hour_mode;

extern "C"
void app_main(void)
{
    init_hardware();

    const auto app_desc = esp_app_get_description();
    printf("AlphaClock v %s\n", app_desc->version);
    
    init_nvs();

    const auto wifi_creds = get_wifi_creds();
    if (!wifi_creds.empty())
    {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());

        int attempts_left = 2;
        bool connected = false;
        while (!connected && attempts_left)
        {
            connected = connect(wifi_creds);
            ESP_LOGI(TAG, "Connected: %d attempts: %d",
                     connected, attempts_left);
            if (!connected)
            {
                disconnect();
                vTaskDelay(10000 / portTICK_PERIOD_MS);
                --attempts_left;
            }
        }
        if (connected)
        {
            ESP_LOGI(TAG, "Connected to WiFi");
            ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

            initialize_sntp();
        }
    }
    
    printf("\n\nPress a key to enter console\n");
    bool debug = false;
    for (int i = 0; i < 20; ++i)
    {
        if (getchar() != EOF)
        {
            debug = true;
            break;
        }
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
    if (debug)
        run_console();        // never returns

    printf("Starting app\n");
    
    esp_log_level_set("esp_wifi", ESP_LOG_ERROR);
    esp_log_level_set("wifi", ESP_LOG_ERROR);

    start_webserver();

    extern bool is_button_pressed;
    bool was_button_pressed = false;

    Stepper* steppers[] = {
        &s_hours, &s_minutes, &s_seconds
    };

    while (1)
    {
        vTaskDelay(1);
        switch (active_mode)
        {
        case MODE_MANUAL:
            if (is_button_pressed)
            {
                int delay = is_button_fast ? 2000 : 15000;
                if (!was_button_pressed)
                    was_button_pressed = true;
                ESP_LOGI(TAG, "Active: %d", active_button);
                auto stepper = steppers[active_button];
                stepper->start(button_direction_up, delay);
            }
            else
            {
                if (was_button_pressed)
                {
                    auto stepper = steppers[active_button];
                    stepper->stop();
                    was_button_pressed = false;
                }
            }
            break;

        case MODE_NORMAL:
            handle_normal_mode();
            break;
            
        case MODE_FAST:
            handle_fast_mode();
            break;
        }
    }
}

void handle_normal_mode()
{
    static time_t last_time = 0;
    
    time_t t;
    time(&t);
    if (t == last_time)
        return;

    last_time = t;
    struct tm tm;
    localtime_r(&t, &tm);

    printf("Normal: %02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

    int fraction = 0;
    if (active_hour_mode == HOUR_MODE_CONTINUOUS)
        fraction = tm.tm_min;
    h_hours.go_to_hour(tm.tm_hour, fraction);
    h_minutes.go_to(tm.tm_min);
    h_seconds.go_to(tm.tm_sec);
}

void handle_fast_mode()
{
    static bool first = true;
    static time_t cur_time = 0;
    static int64_t last_tick = 0;
    
    if (first)
    {
        // Start at current time
        time(&cur_time);
        first = false;
        last_tick = esp_timer_get_time();
        return;
    }

    const auto cur_tick = esp_timer_get_time();
    if (cur_tick - last_tick < 100000)
        return;
    last_tick = cur_tick;
    
    struct tm tm;
    localtime_r(&cur_time, &tm);

    printf("Fast: %02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

    int fraction = 0;
    if (active_hour_mode == HOUR_MODE_CONTINUOUS)
        fraction = tm.tm_min;
    h_hours.go_to_hour(tm.tm_hour, fraction);
    h_minutes.go_to(tm.tm_min);
    h_seconds.go_to(tm.tm_sec);

    ++cur_time;
}

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:
