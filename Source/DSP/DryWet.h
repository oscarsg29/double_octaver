/*
  ==============================================================================

    DryWet.h
    Created: 2 May 2026 10:12:44am
    Author:  Jesus Valdez

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class DryWet
{
public:
    
    DryWet();
    ~DryWet();
    
    void process (juce::AudioBuffer<float>& dryBuffer, juce::AudioBuffer<float>& wetBuffer);
    
    void setDryWetValue (float dryWetValue);
    
private:
    
    float dryWetValue { 0.5f };
    
};
