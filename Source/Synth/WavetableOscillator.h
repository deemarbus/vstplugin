#pragma once

#include <JuceHeader.h>

class WavetableOscillator
{
public:
    void reset() noexcept;
    void setSampleRate (double newSampleRate) noexcept;
    float renderSample (float frequencyHz, float wavetablePosition) noexcept;

private:
    static constexpr int tableSize = 2048;
    static constexpr int tableCount = 4;

    static const std::array<std::array<float, tableSize>, tableCount>& tables();
    static float interpolateTable (const std::array<float, tableSize>& table, float phase) noexcept;

    double sampleRate = 44100.0;
    float phase = 0.0f;
};
