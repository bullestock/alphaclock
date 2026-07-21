#include "hw.h"
#include "defs.h"

#include "sdkconfig.h"

#include <esp_log.h>

#include <freertos/FreeRTOS.h>

constexpr const char* TAG = "hw";

void init_hardware()
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // Outputs
    io_conf.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set
    io_conf.pin_bit_mask = 
        (1ULL << PIN_I2S_DATA) |
        (1ULL << PIN_I2S_BCK) |
        (1ULL << PIN_I2S_WS);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = 
        (1ULL << PIN_SENSOR_H) |
        (1ULL << PIN_SENSOR_M) |
        (1ULL << PIN_SENSOR_S);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    // GPIO34-39 have no pullup
    //io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));
}

bool is_sensor_activated(int index)
{
    switch (index)
    {
    case 0:
        return gpio_get_level(PIN_SENSOR_H);
    case 1:
        return gpio_get_level(PIN_SENSOR_M);
    case 2:
        return gpio_get_level(PIN_SENSOR_S);
    default:
        ESP_LOGE(TAG, "Bad sensor index: %d", index);
        return false;
    }
}

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:
