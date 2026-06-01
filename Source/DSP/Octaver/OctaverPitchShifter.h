/*
  ==============================================================================

    OctaverPitchShifter.h
    Created: 31 May 2026
    Author:  Oscar Santiago Osorio Mendoza

  ==============================================================================
*/

#pragma once

#include "Octaver.h"
#include "../McPherson/McPhersonPitchShifter.h"
#include "../WangRubberband/WangRubberBandPitchShifter.h"

#include <JuceHeader.h>

class OctaverPitchShifter {
  public:
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

    void setShift(Octaver::Shift shift) noexcept
    {
        shift_ = shift;
        activeAlgorithm_ = Octaver::isHighOctaveShift(shift_)
                               ? Algorithm::rubberBand
                               : Algorithm::mcPherson;
        updateAlgorithmPitch();
    }

    void setShiftFromChoiceIndex(int choiceIndex) noexcept
    {
        switch (choiceIndex)
        {
            case 0: setShift(Octaver::Shift::twoDown); break;
            case 1: setShift(Octaver::Shift::oneDown); break;
            case 2: setShift(Octaver::Shift::oneUp); break;
            case 3: setShift(Octaver::Shift::twoUp); break;
            default: setShift(Octaver::Shift::oneDown); break;
        }
    }

    [[nodiscard]] Octaver::Shift getShift() const noexcept { return shift_; }

    void operator()(juce::AudioBuffer<float>& buffer)
    {
        if (activeAlgorithm_ == Algorithm::rubberBand)
            highOctaveShifter_.process(buffer);
        else
            lowOctaveShifter_.process(buffer);
    }

  private:
    enum class Algorithm {
        mcPherson,
        rubberBand
    };

    void updateAlgorithmPitch() noexcept
    {
        const auto semitones = Octaver::getShiftInSemitones(shift_);

        lowOctaveShifter_.setSemitones(semitones);
        highOctaveShifter_.setSemitones(static_cast<float>(semitones));
    }

    Octaver::Shift shift_{Octaver::Shift::oneDown};
    Algorithm activeAlgorithm_{Algorithm::mcPherson};
    McPhersonPitchShifter lowOctaveShifter_;
    WangRubberBandPitchShifter highOctaveShifter_;
};
