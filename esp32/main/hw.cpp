#include "hw.h"
#include "defs.h"
#include "nvs.h"

#include <esp_log.h>

#include <freertos/FreeRTOS.h>

#include <unistd.h>

void init_hardware()
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // Outputs
    io_conf.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set
    io_conf.pin_bit_mask = 
        (1ULL << PIN_EN1) |
        (1ULL << PIN_EN2) |
        (1ULL << PIN_EN3);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));
}

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:
