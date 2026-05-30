/*
  ==============================================================================

    DryWet.h
    Created: 2 May 2026 10:12:44am
    Author:  Jesus Valdez

  ==============================================================================
*/

#pragma once

#include <algorithm>
#include <JuceHeader.h>

class DryWet {
  public:
    inline static constexpr float MinDryWetPercent = 0.0f;
    inline static constexpr float MaxDryWetPercent = 100.0f;
    inline static constexpr float DefaultDryWetPercent = 50.0f;

    explicit DryWet(float dryWetPercent = DefaultDryWetPercent) noexcept
    {
        setDryWetPercent(dryWetPercent);
    }

    void setDryWetPercent(float dryWetPercent) noexcept
    {
        dryWetPercent_ =
            std::clamp(dryWetPercent, MinDryWetPercent, MaxDryWetPercent);

        wetRatio_ = dryWetPercent_ / MaxDryWetPercent;
    }

    [[nodiscard]] float getDryWetPercent() const noexcept { return dryWetPercent_; }

    [[nodiscard]] float getWetRatio() const noexcept { return wetRatio_; }

    [[nodiscard]] float processSample(float drySample, float wetSample) const noexcept
    {
        return drySample * (1.0f - wetRatio_) + wetSample * wetRatio_;
    }

    void process(juce::AudioBuffer<float>& dryBuffer,
                 juce::AudioBuffer<float>& wetBuffer) const;

  private:
    float dryWetPercent_{DefaultDryWetPercent};
    float wetRatio_{0.5f};
};
