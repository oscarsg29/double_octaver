/*
  ==============================================================================

    DryWet.cpp
    Created: 2 May 2026 10:12:44am
    Author:  Jesus Valdez

  ==============================================================================
*/

#include "DryWet.h"

DryWet::DryWet() {}

DryWet::~DryWet() {}

void DryWet::process (juce::AudioBuffer<float>& dryBuffer, juce::AudioBuffer<float>& wetBuffer)
{
    const auto numChannels = juce::jmin (dryBuffer.getNumChannels(), wetBuffer.getNumChannels());
    const auto numSamples = juce::jmin (dryBuffer.getNumSamples(), wetBuffer.getNumSamples());

    for (int channel = 0; channel < numChannels; ++channel)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            const auto wet = wetBuffer.getSample (channel, i);
            const auto dry = dryBuffer.getSample (channel, i);
            const auto out = dry * (1.0f - dryWetValue) + (dryWetValue * wet);
            
            wetBuffer.setSample (channel, i, out);
        }
    }
}

void DryWet::setDryWetValue (float value)
{
    dryWetValue = juce::jlimit (0.0f, 1.0f, value / 100.0f);
}
