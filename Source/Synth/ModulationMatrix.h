#pragma once

#include <array>
#include <cstddef>

class ModulationMatrix
{
public:
    enum class Source
    {
        none,
        velocity,
        noteNumber,
        ampEnvelope,
        filterEnvelope,
        lfo1,
        lfo2,
        macro1,
        macro2
    };

    enum class Target
    {
        none,
        oscillator1Position,
        oscillator2Position,
        oscillator3Position,
        oscillator4Position,
        oscillator1Level,
        oscillator2Level,
        oscillator3Level,
        oscillator4Level,
        filterCutoff,
        outputGain
    };

    struct Slot
    {
        Source source = Source::none;
        Target target = Target::none;
        float amount = 0.0f;
        bool bipolar = true;
        bool enabled = false;
    };

    static constexpr std::size_t slotCount = 8;

    const std::array<Slot, slotCount>& getSlots() const noexcept { return slots; }
    std::array<Slot, slotCount>& getSlots() noexcept { return slots; }

private:
    std::array<Slot, slotCount> slots {};
};
