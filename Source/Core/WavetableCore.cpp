#include "WavetableCore.h"

#include <algorithm>
#include <cmath>

namespace quadtable
{
namespace
{
constexpr float pi = 3.14159265358979323846f;
constexpr float twoPi = 2.0f * pi;

float lerp (float amount, float start, float end) noexcept
{
    return start + amount * (end - start);
}
} // namespace

WavetableCore::WavetableCore() = default;

void WavetableCore::reset() noexcept
{
    phase = 0.0f;
}

void WavetableCore::setSampleRate (double newSampleRate) noexcept
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
}

float WavetableCore::renderSample (float frequencyHz, float wavetablePosition) noexcept
{
    const auto& wavetable = tables();
    const auto boundedPosition = std::clamp (wavetablePosition, 0.0f, 1.0f);
    const auto framePosition = boundedPosition * static_cast<float> (tableCount - 1);
    const auto lowerFrame = static_cast<std::size_t> (std::floor (framePosition));
    const auto upperFrame = std::min (lowerFrame + 1, tableCount - 1);
    const auto blend = framePosition - static_cast<float> (lowerFrame);

    const auto lowerSample = interpolate (wavetable[lowerFrame], phase);
    const auto upperSample = interpolate (wavetable[upperFrame], phase);
    const auto output = lerp (blend, lowerSample, upperSample);

    phase += std::max (0.0f, frequencyHz) / static_cast<float> (sampleRate);
    phase -= std::floor (phase);

    return output;
}

const WavetableCore::TableSet& WavetableCore::tables()
{
    static const auto generatedTables = []
    {
        TableSet result {};

        for (std::size_t index = 0; index < tableSize; ++index)
        {
            const auto radians = twoPi * static_cast<float> (index) / static_cast<float> (tableSize);
            const auto normalisedPhase = static_cast<float> (index) / static_cast<float> (tableSize);

            result[0][index] = std::sin (radians);
            result[1][index] = 2.0f * normalisedPhase - 1.0f;
            result[2][index] = normalisedPhase < 0.5f ? 1.0f : -1.0f;
            result[3][index] = 1.0f - 4.0f * std::abs (normalisedPhase - 0.5f);
        }

        return result;
    }();

    return generatedTables;
}

float WavetableCore::interpolate (const Table& table, float phase) noexcept
{
    const auto tablePosition = phase * static_cast<float> (tableSize);
    const auto index0 = static_cast<std::size_t> (std::floor (tablePosition)) % tableSize;
    const auto index1 = (index0 + 1) % tableSize;
    const auto fraction = tablePosition - static_cast<float> (index0);

    return lerp (fraction, table[index0], table[index1]);
}
} // namespace quadtable
