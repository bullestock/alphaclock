#include "connect.h"
#include "console.h"
#include "defs.h"
#include "hw.h"
#include "nvs.h"
#include "sntp.h"
#include "stepper.h"

#include <stdio.h>

#include <esp_app_desc.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_wifi.h>

extern "C"
void app_main(void)
{
    init_hardware();

    const auto app_desc = esp_app_get_description();
    printf("BigBro v %s\n", app_desc->version);

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

    // TODO
    Stepper stepper(PIN_A1, PIN_A2, PIN_B1, PIN_B2);

    while (1)
    {
        printf("step\n");
        stepper.step(50, 10000);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:
