#include "PluginEditor.h"
#include "PluginProcessor.h"

QuadTableAudioProcessorEditor::QuadTableAudioProcessorEditor (QuadTableAudioProcessor& processor)
    : juce::GenericAudioProcessorEditor (processor)
{
    setSize (720, 860);
}
