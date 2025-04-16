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
#include <esp_wifi.h>

Stepper hours(PIN_EN1), minutes(PIN_EN2), seconds(PIN_EN3);

extern "C"
void app_main(void)
{
    init_hardware();

    const auto app_desc = esp_app_get_description();
    printf("AlphaClock v %s\n", app_desc->version);

#if 0
    int dir = 1;
    while (1)
    {
        for (int i = 0; i < 5; ++i)
        {
            int64_t delay = 10000 + (10 - i)*10000;
            printf("step %s %.1f ms\n",
                   dir >= 0 ? "forward" : "backward",
                   delay/1000.0);
            hours.step(dir * 100, delay);
            vTaskDelay(10 / portTICK_PERIOD_MS);
            while (hours.busy())
            {
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        dir *= -1;
    }
#endif
    
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

    bool is_running = false;

    extern bool is_button_pressed;
    extern int active_button;
    extern bool button_direction_up;
    
    while (1)
    {
        vTaskDelay(1);
        if (!is_running)
        {
            if (is_button_pressed)
            {
                Stepper* steppers[] = {
                    &hours, &minutes, &seconds
                };
                auto stepper = steppers[active_button];
                // TODO: 1 step does not work
                stepper->step(button_direction_up ? 100 : -1, 5000);
            }
        }
#if 0
        for (int i = 0; i < 8; ++i)
        {
            int64_t delay = 1000 + (10 - i)*500;
            printf("step %.1f ms\n", delay/1000.0);
            hours.step(100, delay);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        for (int i = 0; i < 8; ++i)
        {
            int64_t delay = 1000 + (10 - i)*500;
            printf("step %.1f ms\n", delay/1000.0);
            minutes.step(100, delay);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        for (int i = 0; i < 8; ++i)
        {
            int64_t delay = 1000 + (10 - i)*500;
            printf("step %.1f ms\n", delay/1000.0);

            seconds.step(100, delay);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
#endif
    }
}

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:
