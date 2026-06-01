/*
  ==============================================================================

    Processor.h
    Created: 29 May 2026 1:13:44pm
    Author:  Oscar Santiago Osorio Mendoza

  ==============================================================================
*/

#pragma once

#include "ChannelsView/AudioBuffersView.h"

namespace dsp {

template <typename Processor>
void transformSamples(audio::MonoBufferView view, Processor &&processor) noexcept {
  if (!view.isValid())
    return;

  for (int i = 0; i < view.numSamples; ++i) {
    view.samples[i] = processor(view.samples[i]);
  }
}

template <typename Processor>
void transformSamples(audio::StereoBufferView view, Processor &&processor) noexcept {
  if (!view.isValid())
    return;

  transformSamples(view.left, processor);
  transformSamples(view.right, processor);
}

template <typename Processor>
void combineSamples(audio::MonoBufferView output,
                    audio::MonoBufferView inputA,
                    audio::MonoBufferView inputB,
                    Processor &&processor) noexcept {
  if (!output.isValid() || !inputA.isValid() || !inputB.isValid())
    return;

  for (int i = 0; i < output.numSamples; ++i) {
    output.samples[i] = processor(inputA.samples[i], inputB.samples[i]);
  }
}

template <typename Processor>
void combineSamples(audio::StereoBufferView output,
                    audio::StereoBufferView inputA,
                    audio::StereoBufferView inputB,
                    Processor &&processor) noexcept {
  if (!output.isValid() || !inputA.isValid() || !inputB.isValid())
    return;

  combineSamples(output.left, inputA.left, inputB.left, processor);
  combineSamples(output.right, inputA.right, inputB.right, processor);
}

} // namespace dsp
