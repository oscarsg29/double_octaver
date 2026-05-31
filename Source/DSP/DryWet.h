/*
  ==============================================================================

    DryWet.h
    Created: 2 May 2026 10:12:44am
    Author:  Jesus Valdez

  ==============================================================================
*/

#pragma once

#include <algorithm>

class DryWet {
public:
    inline static constexpr float MinDryWetPercent = 0.0f;
    inline static constexpr float MaxDryWetPercent = 100.0f;
    inline static constexpr float DefaultDryWetPercent = 50.0f;

    explicit DryWet(float dryWetPercent = DefaultDryWetPercent) noexcept {
        setDryWetPercent(dryWetPercent);
    }

    void setDryWetPercent(float dryWetPercent) noexcept {
        dryWetPercent_ =
            std::clamp(dryWetPercent, MinDryWetPercent, MaxDryWetPercent);

        wetRatio_ = dryWetPercent_ / MaxDryWetPercent;
        dryRatio_ = 1.0f - wetRatio_;
    }

    [[nodiscard]] float getDryWetPercent() const noexcept {
        return dryWetPercent_;
    }

    [[nodiscard]] float getWetRatio() const noexcept {
        return wetRatio_;
    }

    [[nodiscard]] float getDryRatio() const noexcept {
        return dryRatio_;
    }

    [[nodiscard]] float operator()(float drySample,
                                      float wetSample) const noexcept {
        return drySample * dryRatio_ + wetSample * wetRatio_;
    }

private:
    float dryWetPercent_{DefaultDryWetPercent};
    float wetRatio_{0.5f};
    float dryRatio_{0.5f};
};
