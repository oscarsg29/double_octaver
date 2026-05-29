/*
  ==============================================================================

    LFO.h
    Created: 25 Apr 2026 6:26:37pm
    Author:  andre

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class LFO
{
public:
	LFO();
	~LFO();

	void prepare (double inSampleRate);
	void process (juce::AudioBuffer<float>& buffer);
	void setFrequencyValue (float inFrequency);

private:
	float sampleRate { 44100.0 };
	float frequencyValue{ 20.0f };
	float twoPi = juce::MathConstants<float>::twoPi;
	float time[2];
	float deltaTime[2];
};