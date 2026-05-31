/*
  ==============================================================================

    Panning.h
    Created: 25 Apr 2026 5:26:34pm
    Author:  andre

  ==============================================================================
*/

#pragma once

class Panning
{
public:
    explicit Panning(float p = 0.0f) noexcept
    {
        setPan(p);
    }

    void setPan(float p) noexcept
    {
        pan = std::clamp(p, -1.0f, 1.0f);

        const float angle = (pan + 1.0f) * 0.25f * 3.1415926535f;

        leftGain  = std::cos(angle);
        rightGain = std::sin(angle);
    }

    template <typename View>
    void operator()(View& v, int i) const noexcept
    {
        v.channel(0, i) *= leftGain;
        v.channel(1, i) *= rightGain;
    }

private:
    float pan = 0.0f;
    float leftGain = 1.0f;
    float rightGain = 1.0f;
};
