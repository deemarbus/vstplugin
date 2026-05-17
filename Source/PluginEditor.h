#pragma once

#include <JuceHeader.h>

class QuadTableAudioProcessor;

class QuadTableAudioProcessorEditor final : public juce::GenericAudioProcessorEditor
{
public:
    explicit QuadTableAudioProcessorEditor (QuadTableAudioProcessor& processor);
};
