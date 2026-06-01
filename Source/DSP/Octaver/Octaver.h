/*
  ==============================================================================

    Octaver.h
    Created: 29 May 2026
    Author:  Oscar Santiago Osorio Mendoza

  ==============================================================================
*/

#pragma once

#include "../McPherson/McPhersonPitchShifter.h"
#include "../WangRubberband/WangRubberBandPitchShifter.h"
#include <JuceHeader.h>
#include <algorithm>
#include <cmath>

class Octaver {
  public:
    inline static constexpr float MinOctaveGainDb = -60.0f;
    inline static constexpr float MaxOctaveGainDb = 12.0f;
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
        setShift(Shift::oneDown);
    }

    void prepare(double sampleRate, int maximumBlockSize, int numChannels)
    {
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32>(maximumBlockSize);
        spec.numChannels = static_cast<juce::uint32>(numChannels);

        lowOctaveShifter_.prepare(spec);
        highOctaveShifter_.prepare(spec);
        updateAlgorithmPitch();
    }

    void setOctaveGainDb(float gainDb) noexcept
    {
        octaveGainDb_ = std::clamp(gainDb, MinOctaveGainDb, MaxOctaveGainDb);
        octaveGainLinear_ = std::pow(10.0f, octaveGainDb_ / 20.0f);
    }

    void setShift(Shift shift) noexcept
    {
        shift_ = shift;
        activeAlgorithm_ = isHighOctaveShift(shift_) ? Algorithm::rubberBand : Algorithm::mcPherson;
        updateAlgorithmPitch();
    }

    void setShiftFromChoiceIndex(int choiceIndex) noexcept
    {
        switch (choiceIndex)
        {
            case 0: setShift(Shift::twoDown); break;
            case 1: setShift(Shift::oneDown); break;
            case 2: setShift(Shift::oneUp); break;
            case 3: setShift(Shift::twoUp); break;
            default: setShift(Shift::oneDown); break;
        }
    }

    [[nodiscard]] float getOctaveGainDb() const noexcept { return octaveGainDb_; }

    [[nodiscard]] float getLinearOctaveGain() const noexcept { return octaveGainLinear_; }

    [[nodiscard]] Shift getShift() const noexcept { return shift_; }

    void process(juce::AudioBuffer<float>& buffer)
    {
        if (activeAlgorithm_ == Algorithm::rubberBand)
            highOctaveShifter_.process(buffer);
        else
            lowOctaveShifter_.process(buffer);

        buffer.applyGain(octaveGainLinear_);
    }

  private:
    enum class Algorithm {
        mcPherson,
        rubberBand
    };

    [[nodiscard]] static bool isHighOctaveShift(Shift shift) noexcept
    {
        return shift == Shift::oneUp || shift == Shift::twoUp;
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

    void updateAlgorithmPitch() noexcept
    {
        const auto semitones = getShiftInSemitones(shift_);

        lowOctaveShifter_.setSemitones(semitones);
        highOctaveShifter_.setSemitones(static_cast<float>(semitones));
    }

    Shift shift_{Shift::oneDown};
    Algorithm activeAlgorithm_{Algorithm::mcPherson};
    float octaveGainDb_{DefaultOctaveGainDb};
    float octaveGainLinear_{1.0f};
    McPhersonPitchShifter lowOctaveShifter_;
    WangRubberBandPitchShifter highOctaveShifter_;
};
