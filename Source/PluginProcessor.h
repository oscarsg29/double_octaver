#pragma once

#include "DSP/ChannelsView/MonoPolicy.h"
#include "DSP/ChannelsView/StereoPolicy.h"
#include "DSP/DryWet.h"
#include "DSP/Gain/Gain.h"
#include "DSP/Octaver/Octaver.h"
#include "DSP/SamplesProcessor.h"
#include <JuceHeader.h>
#include <memory>

namespace double_octaver::pitch
{
class OctaveVoiceProcessor;
}

class DoubleOctaverAudioProcessor : public juce::AudioProcessor {
  public:
    DoubleOctaverAudioProcessor();
    ~DoubleOctaverAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;

    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState() noexcept;
    const juce::AudioProcessorValueTreeState& getValueTreeState() const noexcept;

  private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    void updateParameters();
    void updateTransportDiscontinuity(int numSamples);
    void resetEffectPathForTransportJump() noexcept;
    void combineAndApplyOutputGain(juce::AudioBuffer<float>& output,
                                   const juce::AudioBuffer<float>& dry,
                                   const juce::AudioBuffer<float>& wet,
                                   int numSamples) noexcept;

    juce::AudioProcessorValueTreeState apvts;
    juce::AudioBuffer<float> dryBuffer;

    std::unique_ptr<double_octaver::pitch::OctaveVoiceProcessor> voice1;
    std::unique_ptr<double_octaver::pitch::OctaveVoiceProcessor> voice2;
    juce::SmoothedValue<float> outputGainLinear { 1.0f };
    juce::SmoothedValue<float> wetMix { DryWet::DefaultDryWetPercent / DryWet::MaxDryWetPercent };
    juce::SmoothedValue<float> powerMix { 1.0f };
    juce::SmoothedValue<float> transportWetFade { 1.0f };
    juce::Optional<int64_t> lastPlayheadSamplePosition;
    bool wasPlaying = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DoubleOctaverAudioProcessor)
};
