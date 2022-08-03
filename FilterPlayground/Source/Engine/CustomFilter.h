/*
  ==============================================================================

    filter.h
    Created: 30 Jul 2022 10:16:34pm
    Author:  Natalia Escalera

  ==============================================================================
*/

#pragma once
#include <math.h>
class CustomFilter
{
 public:
    float SampleRate;
    

    using CoefficientsPtr = typename juce::dsp::IIR::Coefficients<float>::Ptr;
    CoefficientsPtr coefficients;
    
    inline juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> makeCoefficients(double sampleRate, float cFreq)
    {
        DBG("sampleRate:");
        DBG(sampleRate);
        DBG("cFreq:");
        DBG(cFreq);

        juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> coefficients;

        jassert (sampleRate > 0.0);
        jassert (cFreq > 0 && cFreq <= static_cast<float> (sampleRate * 0.5));
        
        float wd = 2 * juce::MathConstants<float>::pi * cFreq;
        float T = 1/sampleRate;
        float wa = (2/T) * std::tan(wd*T/2);
        float g = wa * T/2;
        
        float alpha = g /(1.0 + g);
        coefficients.add(new juce::dsp::IIR::Coefficients<float> (alpha, alpha, alpha, alpha));
        
        return coefficients;
    }
private:
    float R12;
};
