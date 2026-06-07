/*
  ==============================================================================

    Gain.h
    Created: 18 Apr 2026 11:36:18am
    Author:  Jesus Valdez

  ==============================================================================
*/

#pragma once

#include <algorithm>
#include <cmath>

class Gain {
  public:
    inline static constexpr float MinGainDb = -60.0f;
    inline static constexpr float MaxGainDb = 24.0f;
    inline static constexpr float UnityGainDb = 0.0f;

    explicit Gain(float gainDb = UnityGainDb) noexcept { setGainDb(gainDb); }

    void setGainDb(float gainDb) noexcept {
        gainDb_ = std::clamp(gainDb, MinGainDb, MaxGainDb);

        gainLinear_ = std::pow(10.0f, gainDb_ / 20.0f);
    }

    [[nodiscard]] float getGainDb() const noexcept { return gainDb_; }

    [[nodiscard]] float getLinearGain() const noexcept { return gainLinear_; }

    [[nodiscard]] float operator()(float sample) const noexcept {
        return sample * gainLinear_;
    }

  private:
    float gainDb_{UnityGainDb};
    float gainLinear_{1.0f};
};
