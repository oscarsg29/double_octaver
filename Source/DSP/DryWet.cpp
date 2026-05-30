/*
  ==============================================================================

    DryWet.cpp
    Created: 2 May 2026 10:12:44am
    Author:  Jesus Valdez

  ==============================================================================
*/

#include "DryWet.h"

void DryWet::process(juce::AudioBuffer<float>& dryBuffer,
                     juce::AudioBuffer<float>& wetBuffer) const
{
    const auto numChannels = juce::jmin (dryBuffer.getNumChannels(), wetBuffer.getNumChannels());
    const auto numSamples = juce::jmin (dryBuffer.getNumSamples(), wetBuffer.getNumSamples());

    for (int channel = 0; channel < numChannels; ++channel)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            const auto wet = wetBuffer.getSample (channel, i);
            const auto dry = dryBuffer.getSample (channel, i);
            
            wetBuffer.setSample (channel, i, processSample(dry, wet));
        }
    }
}
