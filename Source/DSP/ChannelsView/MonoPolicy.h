//
//  MonoPolicy.h
//  DoubleOctaver
//
//  Created by Oscar Santiago Osorio Mendoza on 29/05/26.
//

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include "AudioBufferViews.hpp"

namespace ChannelLayout
{
    inline constexpr int MonoChannelIndex = 0;
    inline constexpr int MinMonoChannels  = 1;
}

namespace audio
{

struct MonoPolicy
{
    static MonoBufferView makeView(juce::AudioBuffer<float>& buffer) noexcept
    {
        const int numChannels = buffer.getNumChannels();
        const int numSamples  = buffer.getNumSamples();

        jassert(numChannels >= ChannelLayout::MinMonoChannels);

        if (numChannels < ChannelLayout::MinMonoChannels ||
            numSamples <= 0)
        {
            return {};
        }

        return
        {
            buffer.getWritePointer(ChannelLayout::MonoChannelIndex),
            numSamples
        };
    }
};

} // namespace audio