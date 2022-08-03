/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class FilterPlaygroundAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    FilterPlaygroundAudioProcessorEditor (FilterPlaygroundAudioProcessor&);
    ~FilterPlaygroundAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    FilterPlaygroundAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterPlaygroundAudioProcessorEditor)
};
