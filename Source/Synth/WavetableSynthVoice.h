#pragma once

#include <JuceHeader.h>

#include "ModulationMatrix.h"
#include "WavetableOscillator.h"

struct OscillatorParameters
{
    float level = 0.0f;
    float pan = 0.0f;
    float coarseTuneSemitones = 0.0f;
    float fineTuneCents = 0.0f;
    float wavetablePosition = 0.0f;
};

struct EnvelopeParameters
{
    float attack = 0.01f;
    float decay = 0.2f;
    float sustain = 0.8f;
    float release = 0.35f;
};

struct VoiceParameters
{
    static constexpr size_t oscillatorCount = 4;

    std::array<OscillatorParameters, oscillatorCount> oscillators {};
    EnvelopeParameters ampEnvelope;
    EnvelopeParameters filterEnvelope;
    float filterCutoffHz = 12000.0f;
    float filterResonance = 0.2f;
    float filterEnvelopeAmount = 0.0f;
    float outputGainDb = -9.0f;
    ModulationMatrix modulationMatrix;
};

class WavetableSynthSound final : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int midiNoteNumber) override;
    bool appliesToChannel (int midiChannel) override;
};

class WavetableSynthVoice final : public juce::SynthesiserVoice
{
public:
    WavetableSynthVoice();

    void prepare (double newSampleRate);
    void setVoiceParameters (const VoiceParameters& newParameters);

    bool canPlaySound (juce::SynthesiserSound* sound) override;
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int newPitchWheelValue) override;
    void controllerMoved (int controllerNumber, int newControllerValue) override;
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

private:
    static float midiNoteToFrequency (int midiNote, float tuneSemitones, float fineCents) noexcept;
    static juce::ADSR::Parameters toAdsrParameters (const EnvelopeParameters& envelope) noexcept;

    std::array<WavetableOscillator, VoiceParameters::oscillatorCount> oscillators;
    juce::ADSR ampEnvelope;
    juce::ADSR filterEnvelope;
    juce::dsp::StateVariableTPTFilter<float> filter;

    VoiceParameters parameters;
    double sampleRate = 44100.0;
    int currentMidiNote = 60;
    float noteVelocity = 0.0f;
};
