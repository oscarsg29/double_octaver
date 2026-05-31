/*
  ==============================================================================

    Processor.h
    Created: 29 May 2026 1:13:44pm
    Author:  Oscar Santiago Osorio Mendoza

  ==============================================================================
*/

#pragma once

namespace dsp {

template <typename View, typename Processor>
void transformSamples(View view, Processor &&processor) noexcept {
  if (!view.isValid())
    return;

  for (int i = 0; i < view.numSamples; ++i) {
    view.samples[i] = processor(view.samples[i]);
  }
}

} // namespace dsp
