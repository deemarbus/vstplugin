#pragma once

#include <array>
#include <cstddef>

namespace quadtable
{
class WavetableCore
{
public:
    static constexpr std::size_t tableSize = 2048;
    static constexpr std::size_t tableCount = 4;

    WavetableCore();

    void reset() noexcept;
    void setSampleRate (double newSampleRate) noexcept;
    float renderSample (float frequencyHz, float wavetablePosition) noexcept;

private:
    using Table = std::array<float, tableSize>;
    using TableSet = std::array<Table, tableCount>;

    static const TableSet& tables();
    static float interpolate (const Table& table, float phase) noexcept;

    double sampleRate = 44100.0;
    float phase = 0.0f;
};
} // namespace quadtable
