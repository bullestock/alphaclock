#pragma once

#include <mutex>
#include <string>
#include <vector>

#include <ssd1306.h>

class Display
{
public:
    static Display& instance();

    void clear();

    SSD1306_t* device();

    /// Add progress message
    void add_progress(const std::string& status);

private:
    Display();

    void thread_body();
    
    SSD1306_t display;
    // Used by add_progress()
    int row = 0;
    std::vector<std::string> lines;
};

// Local Variables:
// compile-command: "(cd ..; idf.py build)"
// End:
