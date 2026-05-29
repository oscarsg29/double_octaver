/*
  ==============================================================================

    Filters.cpp
    Created: 9 May 2026 10:32:07am
    Author:  Rodolfo Ortiz

  ==============================================================================
*/

#include "Filters.h"

Filters::Filters() {}
Filters::~Filters() {}

void Filters::prepare (juce::dsp::ProcessSpec& spec)
{
    // Lowpass Filter
    lowpassCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (spec.sampleRate, 1000.0f);
    for (auto& filter : lowpassFilter)
    {
        filter.prepare (spec);
        filter.coefficients = lowpassCoefficients;
    }
    
    // Highpass Filter
    highpassFilter.prepare (spec);
    *highpassFilter.state = juce::dsp::IIR::ArrayCoefficients<float>::makeHighPass (spec.sampleRate, 500.0f);
    
    //Bandpass Filter
    bandpassFilter.prepare (spec);
    bandpassFilter.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
    bandpassFilter.setCutoffFrequency (2000.0f);
}

void Filters::processLowpass (juce::AudioBuffer<float>& buffer)
{
    for (int channel = 0; channel < buffer.getNumChannels(); channel++)
    {
        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            float sample = buffer.getSample (channel, i);
            float outSample = lowpassFilter[channel].processSample (sample);
            buffer.setSample (channel, i, outSample);
        }
    }
}

void Filters::processHighpass (juce::AudioBuffer<float>& buffer)
{
    auto audioBlock = juce::dsp::AudioBlock<float> (buffer);
    auto context = juce::dsp::ProcessContextReplacing<float> (audioBlock);
    
    highpassFilter.process (context);
}

void Filters::processBandpass (juce::AudioBuffer<float>& buffer)
{
    auto audioBlock = juce::dsp::AudioBlock<float> (buffer);
    auto context = juce::dsp::ProcessContextReplacing<float> (audioBlock);
    
    bandpassFilter.process (context);
}

