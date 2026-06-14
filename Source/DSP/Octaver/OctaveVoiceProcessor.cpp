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
}

void OctaveVoiceProcessor::update(float gainDb, bool shouldBypass, int shiftChoiceIndex)
{
    gain.setOctaveGainDb(gainDb);
    bypassed = shouldBypass;
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

    if (bypassed)
    {
        buffer.clear();
        return;
    }

    pitchShifter(buffer);

    if (buffer.getNumChannels() == 1)
    {
        auto view = audio::MonoPolicy::makeView(buffer);
        dsp::transformSamples(view, gain);
    }
    else if (buffer.getNumChannels() == 2)
    {
        auto view = audio::StereoPolicy::makeView(buffer);
        dsp::transformSamples(view, gain);
    }
}
}
