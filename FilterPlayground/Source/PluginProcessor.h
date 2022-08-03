/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Engine/CustomFilter.h"

enum Slope
{
    Slope_6
};
struct ChainSettings
{
    float lowPassFreq {0};
    Slope lowPassSlope {Slope::Slope_6};
    float resonance {1.f};
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& aptvs);

//==============================================================================
/**
*/
class FilterPlaygroundAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    FilterPlaygroundAudioProcessor();
    ~FilterPlaygroundAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Coordinatees gui components with dsp variables
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Parameters", createParameterLayout()};
    
    CustomFilter cFilter;

private:
    
    // Seems to me like we can create an IIR filter and pass it to a processor chain.
    // To change the behavior of the IIR filter, pass the custom coefficients in processBlock
    // as per https://github.com/juce-framework/JUCE/blob/2b16c1b94c90d0db3072f6dc9da481a9484d0435/modules/juce_dsp/processors/juce_IIRFilter.h#L313
    using Filter = juce::dsp::IIR::Filter<float>;
    
    // Based on https://youtu.be/i_Iq4_Kd7Rc?t=2008
    //Processor chain, 1 filter for now.

    using CutFilter = juce::dsp::ProcessorChain<Filter>;
    using MonoChain = juce::dsp::ProcessorChain<CutFilter>;
    MonoChain leftChain, rightChain;
    
    //==============================================================================
    
    enum ChainPositions
    {
        LowPass
    };
    using Coefficients = Filter::CoefficientsPtr;
    static void updateCoefficents(Coefficients& old, const Coefficients& replacements);
    
    template<int Index, typename ChainType, typename CoefficientType>
    void update(ChainType& chain, const CoefficientType& coefficients);
    
    void updateFilters();
    
    template<typename ChainType, typename CoefficientType>
    void updateFilter(ChainType& type,
                         const CoefficientType& coefficients,
                         const Slope& slope );
    
    void updateLowPassFilter(const ChainSettings& chainSettings);


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterPlaygroundAudioProcessor)
};
