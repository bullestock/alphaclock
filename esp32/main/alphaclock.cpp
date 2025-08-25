#include "connect.h"
#include "console.h"
#include "defs.h"
#include "hw.h"
#include "nvs.h"
#include "sntp.h"
#include "stepper.h"
#include "websocket.h"

#include <stdio.h>

#include <esp_app_desc.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <esp_wifi.h>

Stepper hours(PIN_EN1), minutes(PIN_EN2), seconds(PIN_EN3);

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

        int attempts_left = 5;
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
    extern int active_button;
    extern bool button_direction_up;
    extern bool is_button_fast;
    extern Mode active_mode;

    Stepper* steppers[] = {
        &hours, &minutes, &seconds
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

        default:
            break;
        }
    }
}

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:
