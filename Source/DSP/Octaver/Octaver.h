/*
  ==============================================================================

    Octaver.h
    Created: 29 May 2026
    Author:  Oscar Santiago Osorio Mendoza

  ==============================================================================
*/

#pragma once

#include <algorithm>
#include <cmath>

class Octaver {
  public:
    inline static constexpr float MinOctaveGainDb = -60.0f;
    inline static constexpr float MaxOctaveGainDb = 24.0f;
    inline static constexpr float DefaultOctaveGainDb = 0.0f;

    enum class Shift {
        twoDown = 0,
        oneDown,
        oneUp,
        twoUp
    };

    Octaver() noexcept
    {
        setOctaveGainDb(DefaultOctaveGainDb);
    }

    void setOctaveGainDb(float gainDb) noexcept
    {
        octaveGainDb_ = std::clamp(gainDb, MinOctaveGainDb, MaxOctaveGainDb);
        octaveGainLinear_ = std::pow(10.0f, octaveGainDb_ / 20.0f);
    }

    [[nodiscard]] float getOctaveGainDb() const noexcept { return octaveGainDb_; }

    [[nodiscard]] float getLinearOctaveGain() const noexcept { return octaveGainLinear_; }

    [[nodiscard]] float operator()(float sample) const noexcept
    {
        return sample * octaveGainLinear_;
    }

    [[nodiscard]] static bool usesMcPhersonAlgorithm(Shift shift) noexcept
    {
        return shift == Shift::oneDown;
    }

    [[nodiscard]] static int getShiftInSemitones(Shift shift) noexcept
    {
        switch (shift)
        {
            case Shift::twoDown: return -24;
            case Shift::oneDown: return -12;
            case Shift::oneUp:   return 12;
            case Shift::twoUp:   return 24;
        }

        return -12;
    }

  private:
    float octaveGainDb_{DefaultOctaveGainDb};
    float octaveGainLinear_{1.0f};
};
