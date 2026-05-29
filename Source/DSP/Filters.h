#pragma once

#include <JuceHeader.h>

class Filters
{
public:
    Filters();
    ~Filters();
    
    void prepare (juce::dsp::ProcessSpec& spec);
    void processLowpass (juce::AudioBuffer<float>& buffer);
    void processHighpass (juce::AudioBuffer<float>& buffer);
    void processBandpass (juce::AudioBuffer<float>& buffer);
    
private:
    //Lowpass Filter
    std::array<juce::dsp::IIR::Filter<float>, 2> lowpassFilter;
    typename juce::dsp::IIR::Coefficients<float>::Ptr lowpassCoefficients;
    
    //Highpass Filter
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> highpassFilter;
    
    //Bandpass Filter
    juce::dsp::StateVariableTPTFilter<float> bandpassFilter;
};
