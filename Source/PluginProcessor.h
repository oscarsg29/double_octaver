/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once
//#include "DSP/Biquad.h"
#include "DSP/ChannelsView/MonoPolicy.h"
#include "DSP/ChannelsView/StereoPolicy.h"
#include "DSP/DryWet.h"
//#include "DSP/Filters.h"
#include "DSP/Gain/Gain.h"
//#include "DSP/LFO/LFO.h"
#include "DSP/Octaver/Octaver.h"
//#include "DSP/Panning.h"
#include "DSP/SamplesProcessor.h"
#include <JuceHeader.h>
#include <memory>

//==============================================================================
/**
 */
class Curso032026AudioProcessor : public juce::AudioProcessor {
  public:
    //==============================================================================
    Curso032026AudioProcessor();
    ~Curso032026AudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    //==============================================================================
    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

  private:
    class OctaverPitchShifter;

    void updateParameters();

    juce::AudioBuffer<float> dryBuffer;
    juce::AudioBuffer<float> octaveBuffer;

    Gain gain{};
    Octaver octaver{};
    std::unique_ptr<OctaverPitchShifter> octaverPitchShifter;
    // Panning panning;
    // LFO lfo;
    DryWet drywet;
    // LPF_Biquad lpfBiquad;
    // Filters filters;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Curso032026AudioProcessor)
};
