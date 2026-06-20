#include "OctaveVoiceProcessor.h"
#include "../SamplesProcessor.h"
#include "../ChannelsView/MonoPolicy.h"
#include "../ChannelsView/StereoPolicy.h"

namespace double_octaver::pitch
{
void OctaveVoiceProcessor::prepare(double sampleRate, int maximumBlockSize, int numChannels)
{
    buffer.setSize(numChannels, maximumBlockSize, false, false, true);
    pitchShifter.prepare(sampleRate, maximumBlockSize, numChannels);
    gainLinear.reset(sampleRate, 0.02);
    gainLinear.setCurrentAndTargetValue(1.0f);
    activeMix.reset(sampleRate, 0.03);
    activeMix.setCurrentAndTargetValue(1.0f);
}

void OctaveVoiceProcessor::reset() noexcept
{
    pitchShifter.reset();
    activeMix.setCurrentAndTargetValue(0.0f);
    activeMix.setTargetValue(bypassed ? 0.0f : 1.0f);
}

void OctaveVoiceProcessor::update(float gainDb, bool shouldBypass, int shiftChoiceIndex)
{
    const auto clampedGainDb = std::clamp(gainDb, Octaver::MinOctaveGainDb, Octaver::MaxOctaveGainDb);
    gainLinear.setTargetValue(std::pow(10.0f, clampedGainDb / 20.0f));
    bypassed = shouldBypass;
    activeMix.setTargetValue(bypassed ? 0.0f : 1.0f);
    pitchShifter.setShiftFromChoiceIndex(shiftChoiceIndex);
}

void OctaveVoiceProcessor::process(const juce::AudioBuffer<float>& input, int numSamples)
{
    jassert(input.getNumChannels() == buffer.getNumChannels());
    jassert(numSamples <= buffer.getNumSamples());

    if (input.getNumChannels() != buffer.getNumChannels() || numSamples > buffer.getNumSamples())
    {
        buffer.clear();
        return;
    }

    for (auto channel = 0; channel < input.getNumChannels(); ++channel)
        buffer.copyFrom(channel, 0, input, channel, 0, numSamples);

    pitchShifter(buffer);
    applyGain(numSamples);
    applyBypassFade(numSamples);
}

void OctaveVoiceProcessor::applyGain(int numSamples) noexcept
{
    if (! gainLinear.isSmoothing())
    {
        buffer.applyGain(0, numSamples, gainLinear.getCurrentValue());
        return;
    }

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto gain = gainLinear.getNextValue();

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample(channel, sample, buffer.getSample(channel, sample) * gain);
    }
}

void OctaveVoiceProcessor::applyBypassFade(int numSamples) noexcept
{
    if (! activeMix.isSmoothing())
    {
        buffer.applyGain(0, numSamples, activeMix.getCurrentValue());
        return;
    }

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto mix = activeMix.getNextValue();

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample(channel, sample, buffer.getSample(channel, sample) * mix);
    }
}
}
