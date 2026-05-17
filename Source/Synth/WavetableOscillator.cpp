#include "WavetableOscillator.h"

void WavetableOscillator::reset() noexcept
{
    phase = 0.0f;
}

void WavetableOscillator::setSampleRate (double newSampleRate) noexcept
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
}

float WavetableOscillator::renderSample (float frequencyHz, float wavetablePosition) noexcept
{
    const auto& wavetable = tables();
    const auto boundedPosition = juce::jlimit (0.0f, 1.0f, wavetablePosition);
    const auto framePosition = boundedPosition * static_cast<float> (tableCount - 1);
    const auto lowerFrame = static_cast<int> (std::floor (framePosition));
    const auto upperFrame = juce::jmin (lowerFrame + 1, tableCount - 1);
    const auto blend = framePosition - static_cast<float> (lowerFrame);

    const auto lowerSample = interpolateTable (wavetable[static_cast<size_t> (lowerFrame)], phase);
    const auto upperSample = interpolateTable (wavetable[static_cast<size_t> (upperFrame)], phase);
    const auto output = juce::jmap (blend, lowerSample, upperSample);

    phase += frequencyHz / static_cast<float> (sampleRate);
    phase -= std::floor (phase);

    return output;
}

const std::array<std::array<float, WavetableOscillator::tableSize>, WavetableOscillator::tableCount>& WavetableOscillator::tables()
{
    static const auto generatedTables = []
    {
        std::array<std::array<float, tableSize>, tableCount> result {};

        for (int index = 0; index < tableSize; ++index)
        {
            const auto phaseRadians = juce::MathConstants<float>::twoPi * static_cast<float> (index) / static_cast<float> (tableSize);
            const auto normalisedPhase = static_cast<float> (index) / static_cast<float> (tableSize);

            result[0][static_cast<size_t> (index)] = std::sin (phaseRadians);
            result[1][static_cast<size_t> (index)] = 2.0f * normalisedPhase - 1.0f;
            result[2][static_cast<size_t> (index)] = normalisedPhase < 0.5f ? 1.0f : -1.0f;
            result[3][static_cast<size_t> (index)] = 1.0f - 4.0f * std::abs (normalisedPhase - 0.5f);
        }

        return result;
    }();

    return generatedTables;
}

float WavetableOscillator::interpolateTable (const std::array<float, tableSize>& table, float phase) noexcept
{
    const auto tablePosition = phase * static_cast<float> (tableSize);
    const auto index0 = static_cast<int> (std::floor (tablePosition)) % tableSize;
    const auto index1 = (index0 + 1) % tableSize;
    const auto fraction = tablePosition - static_cast<float> (index0);

    return juce::jmap (fraction, table[static_cast<size_t> (index0)], table[static_cast<size_t> (index1)]);
}
