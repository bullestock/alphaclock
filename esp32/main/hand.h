#pragma once

#include "defs.h"

class Stepper;

class Hand
{
public:
    Hand(Stepper& m);

    void zero();
    
    // Move to position 0-59
    void go_to(int position);

    // Move to remapped hour 0-12
    void go_to_hour(int hour);

private:
    Stepper& motor;

    // In steps (0 through N-1, where N is steps/revolution)
    int current_position = 0;
};

// Local Variables:
// compile-command: "(cd ..; idf.py build)"
// End:
