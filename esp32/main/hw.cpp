#include "hw.h"
#include "defs.h"

#include <esp_log.h>

#include <freertos/FreeRTOS.h>

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
}

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:
