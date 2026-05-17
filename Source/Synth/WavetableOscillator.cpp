#include "WavetableOscillator.h"

void WavetableOscillator::reset() noexcept
{
    core.reset();
}

void WavetableOscillator::setSampleRate (double newSampleRate) noexcept
{
    core.setSampleRate (newSampleRate);
}

float WavetableOscillator::renderSample (float frequencyHz, float wavetablePosition) noexcept
{
    return core.renderSample (frequencyHz, wavetablePosition);
}
