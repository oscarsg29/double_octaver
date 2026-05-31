//
//  StereoPolicy.h
//  DoubleOctaver
//
//  Created by Oscar Santiago Osorio Mendoza on 29/05/26.
//

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include "AudioBuffersView.h"

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

        float* const leftSamples =
            buffer.getWritePointer(ChannelLayout::LeftChannelIndex);

        float* const rightSamples =
            (numChannels > ChannelLayout::RightChannelIndex)
                ? buffer.getWritePointer(ChannelLayout::RightChannelIndex)
                : leftSamples; // mono fallback

        return
        {
            {leftSamples, numSamples},
            {rightSamples, numSamples},
            numSamples
        };
    }
};

} // namespace audio
