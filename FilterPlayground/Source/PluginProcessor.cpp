/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FilterPlaygroundAudioProcessor::FilterPlaygroundAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

FilterPlaygroundAudioProcessor::~FilterPlaygroundAudioProcessor()
{
}

//==============================================================================
const juce::String FilterPlaygroundAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FilterPlaygroundAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FilterPlaygroundAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FilterPlaygroundAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FilterPlaygroundAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FilterPlaygroundAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FilterPlaygroundAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FilterPlaygroundAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FilterPlaygroundAudioProcessor::getProgramName (int index)
{
    return {};
}

void FilterPlaygroundAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FilterPlaygroundAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::dsp::ProcessSpec spec;
    
    spec.maximumBlockSize = samplesPerBlock;
    
    spec.numChannels = 1;
    
    spec.sampleRate = sampleRate;
    
    leftChain.prepare(spec);
    rightChain.prepare(spec);
    
    updateFilters();
}

void FilterPlaygroundAudioProcessor::updateFilters()
{
    auto chainSettings = getChainSettings(apvts);
    updateLowPassFilter(chainSettings);
}


void FilterPlaygroundAudioProcessor::updateLowPassFilter(const ChainSettings &chainSettings)
{
    DBG("updateLowPassFilterFromScratch");
    DBG("chainSettings.lowPassFreq:");
    DBG(chainSettings.lowPassFreq);
    
    // Generating the coefficients
    auto lowPassCoefficients = cFilter.makeCoefficients(getSampleRate(), chainSettings.lowPassFreq);
    
    auto& leftLowPass = leftChain.get<ChainPositions::LowPass>();
    updateFilter(leftLowPass, lowPassCoefficients, chainSettings.lowPassSlope);
    
    auto& rightLowPass = rightChain.get<ChainPositions::LowPass>();
    updateFilter(rightLowPass, lowPassCoefficients, chainSettings.lowPassSlope);
}

template<typename ChainType, typename CoefficientType>
void FilterPlaygroundAudioProcessor::updateFilter(ChainType &type,
                     const CoefficientType &coefficients,
                     const Slope &slope
                     )
{
    type.template setBypassed<0>(true);
    
    update<0>(type, coefficients);
    
}
template<int Index, typename ChainType, typename CoefficientType>
void FilterPlaygroundAudioProcessor::update(ChainType &chain, const CoefficientType &coefficients)
{
    updateCoefficents(chain.template get<Index>().coefficients, coefficients[Index]);
    chain.template setBypassed<Index>(false);
}

void FilterPlaygroundAudioProcessor::updateCoefficents(Coefficients &old, const Coefficients &replacememts)
{
    *old = *replacememts;
}

void FilterPlaygroundAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FilterPlaygroundAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void FilterPlaygroundAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

   
    updateFilters();
    

    juce::dsp::AudioBlock<float> block(buffer);
    
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);
    
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    
    leftChain.process(leftContext);
    rightChain.process(rightContext);
}

//==============================================================================
bool FilterPlaygroundAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FilterPlaygroundAudioProcessor::createEditor()
{
//    return new FilterPlaygroundAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void FilterPlaygroundAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void FilterPlaygroundAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;
    
    settings.lowPassFreq = apvts.getRawParameterValue("LowPass Freq")->load();
    settings.lowPassSlope = static_cast<Slope>(apvts.getRawParameterValue("LowPass Slope")->load());
    settings.resonance = apvts.getRawParameterValue("Resonance")->load();
    
    return settings;
}

juce::AudioProcessorValueTreeState::ParameterLayout FilterPlaygroundAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowPass Freq",
                                                           "LowPass Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                           1000.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("Resonance",
                                                           "Resonance",
                                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
                                                           0.f));
        
    juce::StringArray stringArray;
    for( int i = 0; i < 2; ++i )
    {
        juce::String str;
        str << (6 + i*6);
        str << " db/Oct";
        stringArray.add(str);
    }
    layout.add(std::make_unique<juce::AudioParameterChoice>("LowPass Slope", "LowPass Slope", stringArray, 0));

    return layout;
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FilterPlaygroundAudioProcessor();
}
