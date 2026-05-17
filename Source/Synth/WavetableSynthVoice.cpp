#include "WavetableSynthVoice.h"

bool WavetableSynthSound::appliesToNote (int midiNoteNumber)
{
    juce::ignoreUnused (midiNoteNumber);
    return true;
}

bool WavetableSynthSound::appliesToChannel (int midiChannel)
{
    juce::ignoreUnused (midiChannel);
    return true;
}

WavetableSynthVoice::WavetableSynthVoice()
{
    filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
}

void WavetableSynthVoice::prepare (double newSampleRate)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;

    for (auto& oscillator : oscillators)
        oscillator.setSampleRate (sampleRate);

    ampEnvelope.setSampleRate (sampleRate);
    filterEnvelope.setSampleRate (sampleRate);

    juce::dsp::ProcessSpec spec { sampleRate, 1, 2 };
    filter.prepare (spec);
    filter.reset();
}

void WavetableSynthVoice::setVoiceParameters (const VoiceParameters& newParameters)
{
    parameters = newParameters;
    ampEnvelope.setParameters (toAdsrParameters (parameters.ampEnvelope));
    filterEnvelope.setParameters (toAdsrParameters (parameters.filterEnvelope));
}

bool WavetableSynthVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<WavetableSynthSound*> (sound) != nullptr;
}

void WavetableSynthVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition)
{
    juce::ignoreUnused (sound, currentPitchWheelPosition);

    currentMidiNote = midiNoteNumber;
    noteVelocity = velocity;

    for (auto& oscillator : oscillators)
        oscillator.reset();

    filter.reset();
    ampEnvelope.noteOn();
    filterEnvelope.noteOn();
}

void WavetableSynthVoice::stopNote (float velocity, bool allowTailOff)
{
    juce::ignoreUnused (velocity);

    if (allowTailOff)
    {
        ampEnvelope.noteOff();
        filterEnvelope.noteOff();
        return;
    }

    clearCurrentNote();
    ampEnvelope.reset();
    filterEnvelope.reset();
}

void WavetableSynthVoice::pitchWheelMoved (int newPitchWheelValue)
{
    juce::ignoreUnused (newPitchWheelValue);
}

void WavetableSynthVoice::controllerMoved (int controllerNumber, int newControllerValue)
{
    juce::ignoreUnused (controllerNumber, newControllerValue);
}

void WavetableSynthVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (! isVoiceActive())
        return;

    auto* left = outputBuffer.getWritePointer (0);
    auto* right = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer (1) : nullptr;
    const auto outputGain = juce::Decibels::decibelsToGain (parameters.outputGainDb);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto amp = ampEnvelope.getNextSample();
        const auto filterEnv = filterEnvelope.getNextSample();

        float mixedLeft = 0.0f;
        float mixedRight = 0.0f;

        for (size_t oscIndex = 0; oscIndex < oscillators.size(); ++oscIndex)
        {
            const auto& oscParams = parameters.oscillators[oscIndex];
            if (oscParams.level <= 0.0001f)
                continue;

            const auto frequency = midiNoteToFrequency (currentMidiNote, oscParams.coarseTuneSemitones, oscParams.fineTuneCents);
            const auto oscillatorSample = oscillators[oscIndex].renderSample (frequency, oscParams.wavetablePosition) * oscParams.level;
            const auto leftGain = std::cos ((oscParams.pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f);
            const auto rightGain = std::sin ((oscParams.pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f);

            mixedLeft += oscillatorSample * leftGain;
            mixedRight += oscillatorSample * rightGain;
        }

        const auto envCutoffOffset = parameters.filterEnvelopeAmount * filterEnv * 12000.0f;
        const auto cutoff = juce::jlimit (20.0f, 20000.0f, parameters.filterCutoffHz + envCutoffOffset);
        filter.setCutoffFrequency (cutoff);
        filter.setResonance (juce::jlimit (0.1f, 1.0f, parameters.filterResonance));

        mixedLeft = filter.processSample (0, mixedLeft);
        mixedRight = filter.processSample (1, mixedRight);

        const auto finalLeft = mixedLeft * amp * noteVelocity * outputGain;
        const auto finalRight = mixedRight * amp * noteVelocity * outputGain;
        left[startSample + sample] += finalLeft;

        if (right != nullptr)
            right[startSample + sample] += finalRight;
    }

    if (! ampEnvelope.isActive())
        clearCurrentNote();
}

float WavetableSynthVoice::midiNoteToFrequency (int midiNote, float tuneSemitones, float fineCents) noexcept
{
    const auto note = static_cast<float> (midiNote) + tuneSemitones + fineCents / 100.0f;
    return 440.0f * std::pow (2.0f, (note - 69.0f) / 12.0f);
}

juce::ADSR::Parameters WavetableSynthVoice::toAdsrParameters (const EnvelopeParameters& envelope) noexcept
{
    return {
        juce::jmax (0.001f, envelope.attack),
        juce::jmax (0.001f, envelope.decay),
        juce::jlimit (0.0f, 1.0f, envelope.sustain),
        juce::jmax (0.001f, envelope.release)
    };
}
