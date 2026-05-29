#include "Biquad.h"

LPF_Biquad::LPF_Biquad() {}

LPF_Biquad::~LPF_Biquad() {}

void LPF_Biquad::prepare (double inSampleRate)
{
    sampleRate = inSampleRate;
    updateFilter();
}

void LPF_Biquad::process (juce::AudioBuffer<float> inBuffer)
{
    for (int channel = 0; channel < inBuffer.getNumChannels(); channel++)
    {
        for (int i = 0; i < inBuffer.getNumSamples(); i++)
        {
            float sample = inBuffer.getSample (channel, i);
            float outSample = processSample (sample, channel);
            inBuffer.setSample (channel, i, outSample);
        }
    }
}

float LPF_Biquad::processSample (float inSample, int channel)
{
    float y = 0.0f;
    
    y = (b0 * inSample + b1 * x1[channel] + b2 * x2[channel] + (-a1) * y1[channel] + (-a2) * y2[channel]) * (1.0f / a0);
    
    x2[channel] = x1[channel];
    x1[channel] = inSample;
    y2[channel] = y1[channel];
    y1[channel] = y;
    
    return y;
}

void LPF_Biquad::setFrequency (float inFrequency)
{
    frequency = inFrequency;
    updateFilter();
}

void LPF_Biquad::setQ (float inQ)
{
    Q = inQ;
    updateFilter();
}

void LPF_Biquad::updateFilter()
{
    // Calcular coeficientes
    w0 = (juce::MathConstants<float>::twoPi * frequency) / static_cast<float>(sampleRate);
    alpha = sinf(w0) / (2.0f * Q);
    
    float cosw0 = cosf(w0);
    
    b0 = (1.0f - cosw0) / 2.0f;
    b1 = 1.0f - cosw0;
    b2 = (1.0f - cosw0) / 2.0f;
    
    a0 = 1.0f + alpha;
    a1 = -2.0f * cosw0;
    a2 = 1.0f - alpha;
}
