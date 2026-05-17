#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr int maxVoices = 16;
constexpr int oscillatorCount = 4;

juce::String oscPrefix (int oneBasedOscillatorIndex)
{
    return "osc" + juce::String (oneBasedOscillatorIndex);
}

std::atomic<float>* parameter (const juce::AudioProcessorValueTreeState& state, const juce::String& id)
{
    auto* value = state.getRawParameterValue (id);
    jassert (value != nullptr);
    return value;
}
} // namespace

QuadTableAudioProcessor::QuadTableAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    synth.addSound (new WavetableSynthSound());

    for (int voice = 0; voice < maxVoices; ++voice)
        synth.addVoice (new WavetableSynthVoice());
}

void QuadTableAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);
    synth.setCurrentPlaybackSampleRate (sampleRate);

    for (auto* voice : synth.getVoices())
        if (auto* wavetableVoice = dynamic_cast<WavetableSynthVoice*> (voice))
            wavetableVoice->prepare (sampleRate);
}

void QuadTableAudioProcessor::releaseResources()
{
}

bool QuadTableAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOutput = layouts.getMainOutputChannelSet();
    return mainOutput == juce::AudioChannelSet::mono() || mainOutput == juce::AudioChannelSet::stereo();
}

void QuadTableAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    const auto voiceParameters = readVoiceParameters();
    for (auto* voice : synth.getVoices())
        if (auto* wavetableVoice = dynamic_cast<WavetableSynthVoice*> (voice))
            wavetableVoice->setVoiceParameters (voiceParameters);

    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());
}

juce::AudioProcessorEditor* QuadTableAudioProcessor::createEditor()
{
    return new QuadTableAudioProcessorEditor (*this);
}

bool QuadTableAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String QuadTableAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool QuadTableAudioProcessor::acceptsMidi() const
{
    return true;
}

bool QuadTableAudioProcessor::producesMidi() const
{
    return false;
}

bool QuadTableAudioProcessor::isMidiEffect() const
{
    return false;
}

double QuadTableAudioProcessor::getTailLengthSeconds() const
{
    return 2.0;
}

int QuadTableAudioProcessor::getNumPrograms()
{
    return 1;
}

int QuadTableAudioProcessor::getCurrentProgram()
{
    return 0;
}

void QuadTableAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String QuadTableAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void QuadTableAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

void QuadTableAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = parameters.copyState(); auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void QuadTableAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorValueTreeState::ParameterLayout QuadTableAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    for (int osc = 1; osc <= oscillatorCount; ++osc)
    {
        const auto prefix = oscPrefix (osc);
        params.push_back (std::make_unique<juce::AudioParameterFloat> (prefix + "Level", "Osc " + juce::String (osc) + " Level", juce::NormalisableRange<float> (0.0f, 1.0f), osc == 1 ? 0.8f : 0.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (prefix + "Pan", "Osc " + juce::String (osc) + " Pan", juce::NormalisableRange<float> (-1.0f, 1.0f), 0.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (prefix + "Tune", "Osc " + juce::String (osc) + " Tune", juce::NormalisableRange<float> (-24.0f, 24.0f, 1.0f), 0.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (prefix + "Fine", "Osc " + juce::String (osc) + " Fine", juce::NormalisableRange<float> (-100.0f, 100.0f), 0.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (prefix + "Position", "Osc " + juce::String (osc) + " WT Position", juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    }

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("ampAttack", "Amp Attack", juce::NormalisableRange<float> (0.001f, 5.0f, 0.001f, 0.35f), 0.01f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("ampDecay", "Amp Decay", juce::NormalisableRange<float> (0.001f, 5.0f, 0.001f, 0.35f), 0.2f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("ampSustain", "Amp Sustain", juce::NormalisableRange<float> (0.0f, 1.0f), 0.8f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("ampRelease", "Amp Release", juce::NormalisableRange<float> (0.001f, 8.0f, 0.001f, 0.35f), 0.35f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterCutoff", "Filter Cutoff", juce::NormalisableRange<float> (20.0f, 20000.0f, 0.01f, 0.25f), 12000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterResonance", "Filter Resonance", juce::NormalisableRange<float> (0.1f, 1.0f), 0.2f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterEnvAmount", "Filter Env Amount", juce::NormalisableRange<float> (-1.0f, 1.0f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterAttack", "Filter Attack", juce::NormalisableRange<float> (0.001f, 5.0f, 0.001f, 0.35f), 0.01f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterDecay", "Filter Decay", juce::NormalisableRange<float> (0.001f, 5.0f, 0.001f, 0.35f), 0.25f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterSustain", "Filter Sustain", juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterRelease", "Filter Release", juce::NormalisableRange<float> (0.001f, 8.0f, 0.001f, 0.35f), 0.25f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("outputGain", "Output Gain", juce::NormalisableRange<float> (-36.0f, 6.0f), -9.0f));

    return { params.begin(), params.end() };
}

VoiceParameters QuadTableAudioProcessor::readVoiceParameters() const
{
    VoiceParameters values;

    for (int osc = 0; osc < oscillatorCount; ++osc)
    {
        const auto prefix = oscPrefix (osc + 1);
        auto& target = values.oscillators[static_cast<size_t> (osc)];
        target.level = parameter (parameters, prefix + "Level")->load();
        target.pan = parameter (parameters, prefix + "Pan")->load();
        target.coarseTuneSemitones = parameter (parameters, prefix + "Tune")->load();
        target.fineTuneCents = parameter (parameters, prefix + "Fine")->load();
        target.wavetablePosition = parameter (parameters, prefix + "Position")->load();
    }

    values.ampEnvelope.attack = parameter (parameters, "ampAttack")->load();
    values.ampEnvelope.decay = parameter (parameters, "ampDecay")->load();
    values.ampEnvelope.sustain = parameter (parameters, "ampSustain")->load();
    values.ampEnvelope.release = parameter (parameters, "ampRelease")->load();

    values.filterEnvelope.attack = parameter (parameters, "filterAttack")->load();
    values.filterEnvelope.decay = parameter (parameters, "filterDecay")->load();
    values.filterEnvelope.sustain = parameter (parameters, "filterSustain")->load();
    values.filterEnvelope.release = parameter (parameters, "filterRelease")->load();

    values.filterCutoffHz = parameter (parameters, "filterCutoff")->load();
    values.filterResonance = parameter (parameters, "filterResonance")->load();
    values.filterEnvelopeAmount = parameter (parameters, "filterEnvAmount")->load();
    values.outputGainDb = parameter (parameters, "outputGain")->load();

    return values;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new QuadTableAudioProcessor();
}
