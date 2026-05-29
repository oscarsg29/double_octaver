/*
  ==============================================================================

    LFO.cpp
    Created: 25 Apr 2026 6:26:37pm
    Author:  andre

  ==============================================================================
*/

#include "LFO.h"

LFO::LFO() {}
LFO::~LFO() {}

void LFO::prepare(double inSampleRate) 
{
	sampleRate = static_cast<float> (inSampleRate);

    for (int i = 0; i < 2; i++)
    {
        time[i] = 0.0f;
        deltaTime[i] =  1.0f / sampleRate;
	}
}

void LFO::process(juce::AudioBuffer<float>& buffer) 
{
    for (int channel = 0; channel < buffer.getNumChannels(); channel++)
    {
        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            float sample = buffer.getSample(channel, i);

            float sinValue = (0.5f * std::sinf (twoPi * frequencyValue * time[channel])) + 0.5f;

			float outSample = sample * sinValue;

            buffer.setSample (channel, i, outSample);

			time[channel] += deltaTime[channel];

            if (time[channel] >= 1.0f)
                time[channel] = 0.0f;
        }
    }
}

void LFO::setFrequencyValue(float inFrequency) 
{
	frequencyValue = inFrequency;

}
