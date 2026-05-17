#pragma once

#include "WavetableCore.h"

class WavetableOscillator
{
public:
    void reset() noexcept;
    void setSampleRate (double newSampleRate) noexcept;
    float renderSample (float frequencyHz, float wavetablePosition) noexcept;

private:
    quadtable::WavetableCore core;
};
