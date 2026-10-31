#pragma once

#include "defs.h"

class Stepper;

class Hand
{
public:
    Hand(Stepper& m);

    /// Go to home position
    bool home();
    
    // Move to position 0-59
    void go_to(int position,
               bool wait = true);

    // Move to remapped hour 0-12
    // Fraction is fractional hours (0-59)
    void go_to_hour(int hour, int fraction, bool wait = true);

    // Wait for current operation to complete
    void wait();

    Stepper& get_motor()
    {
        return motor;
    }

private:
    Stepper& motor;
    int current_position = 0; // 0-59
};

extern void set_hands(int hour, int min, int sec);

extern Hand& get_hand(int hand);

extern Hand h_hours;
extern Hand h_minutes;
extern Hand h_seconds;

// Local Variables:
// compile-command: "(cd ..; idf.py build)"
// End:
