#include "WavetableCore.h"

#include <cmath>
#include <iostream>

int main()
{
    quadtable::WavetableCore oscillator;
    oscillator.setSampleRate (48000.0);

    float energy = 0.0f;
    for (int sample = 0; sample < 4800; ++sample)
    {
        const auto value = oscillator.renderSample (440.0f, 0.35f);
        if (! std::isfinite (value))
        {
            std::cerr << "Generated a non-finite sample\n";
            return 1;
        }

        energy += std::abs (value);
    }

    if (energy <= 0.0f)
    {
        std::cerr << "Oscillator generated silence\n";
        return 1;
    }

    return 0;
}
