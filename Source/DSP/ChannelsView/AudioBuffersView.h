/*
  ==============================================================================

 AudioBuffersView.h
    Created: 29 May 2026 12:05:00pm
    Author:  Oscar Santiago Osorio Mendoza

  ==============================================================================
*/

#pragma once

namespace audio {

struct MonoBufferView {
    float *samples{nullptr};
    int numSamples{0};

    [[nodiscard]] bool isValid() const noexcept {
        return samples != nullptr && numSamples > 0;
    }
};

struct StereoBufferView {
    MonoBufferView left{};
    MonoBufferView right{};
    int numSamples{0};

    [[nodiscard]] bool isValid() const noexcept {
        return left.isValid() && right.isValid() && numSamples > 0;
    }
};

} // namespace audio


