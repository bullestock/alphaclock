#pragma once

#include "defs.h"

class Stepper;

class Hand
{
public:
    Hand(Stepper& m);

    static void set_debug(bool on);

    void zero();
    
    // Move to position 0-59
    void go_to(int position,
               bool wait = true);

    // Move to remapped hour 0-12
    // Fraction is fractional hours (0-59)
    void go_to_hour(int hour, int fraction);

    // Wait for current operation to complete
    void wait();

private:
    Stepper& motor;

    // In steps (0 through N-1, where N is steps/revolution)
    int current_position = 0;
};

extern void set_hands(int hour, int min, int sec);

extern Hand h_hours;
extern Hand h_minutes;
extern Hand h_seconds;

// Local Variables:
// compile-command: "(cd ..; idf.py build)"
// End:
