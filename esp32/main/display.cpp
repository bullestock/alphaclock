#include "display.h"
#include "defs.h"
#include "sdkconfig.h"

#include <string.h>

static constexpr const int STATUS_START = 4;

Display& Display::instance()
{
    static Display the_instance;
    return the_instance;
}

Display::Display()
{
	i2c_master_init(&display, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO,
                    CONFIG_RESET_GPIO);

    ssd1306_init(&display, 128, 64);
    ssd1306_contrast(&display, 0xff);
    clear();
}

void Display::clear()
{
    ssd1306_clear_screen(&display, false);
    lines.clear();
    row = 0;
}

SSD1306_t* Display::device()
{
    return &display;
}

void Display::add_progress(const std::string& status)
{
    std::string txt = std::string(" ") + status;
    ssd1306_display_text(device(), row, txt.c_str(), txt.size(), false);
    ++row;
    lines.push_back(status);
    if (row < 7)
        return; // still room for more
    // Out of room, scroll up
    lines.erase(lines.begin());
    --row;
    clear();
    for (int i = 0; i < lines.size(); ++i)
    {
        ssd1306_display_text(device(), i, lines[i].c_str(), lines[i].size(), false);
    }
}

// Local Variables:
// compile-command: "(cd ..; idf.py build)"
// End:
