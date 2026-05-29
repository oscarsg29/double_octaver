//
//  StereoPolicy.h
//  DoubleOctaver
//
//  Created by Oscar Santiago Osorio Mendoza on 29/05/26.
//

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include "AudioBufferViews.hpp"

namespace ChannelLayout
{
    inline constexpr int LeftChannelIndex  = 0;
    inline constexpr int RightChannelIndex = 1;

    inline constexpr int MinStereoChannels = 1;
}

namespace audio
{

struct StereoPolicy
{
    [[nodiscard]]
    static StereoBufferView makeView(juce::AudioBuffer<float>& buffer) noexcept
    {
        const int numChannels = buffer.getNumChannels();
        const int numSamples  = buffer.getNumSamples();

        jassert(numChannels >= ChannelLayout::MinStereoChannels);

        if (numChannels < ChannelLayout::MinStereoChannels ||
            numSamples <= 0)
        {
            return {};
        }

        float* const left =
            buffer.getWritePointer(ChannelLayout::LeftChannelIndex);

        float* const right =
            (numChannels > ChannelLayout::RightChannelIndex)
                ? buffer.getWritePointer(ChannelLayout::RightChannelIndex)
                : left; // mono fallback

        return
        {
            left,
            right,
            numSamples
        };
    }
};

} // namespace audio