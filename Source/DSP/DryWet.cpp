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
    for (int channel = 0; channel < dryBuffer.getNumChannels(); channel++)
    {
        for (int i = 0; i < dryBuffer.getNumSamples(); i++)
        {
            float wet = wetBuffer.getSample (channel, i);
            float dry = dryBuffer.getSample (channel, i);
            float out = dry * (1.0f - dryWetValue) + (dryWetValue * wet);
            
            wetBuffer.setSample (channel, i, out);
        }
    }
}

void DryWet::setDryWetValue (float value)
{
    dryWetValue = value;
    dryWetValue = dryWetValue / 100.0f;
}
