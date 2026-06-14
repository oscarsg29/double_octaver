#pragma once

#include "Octaver.h"
#include <JuceHeader.h>

namespace double_octaver::pitch
{
class OctaverPitchShifter
{
public:
    OctaverPitchShifter();
    ~OctaverPitchShifter();

    void prepare(double sampleRate, int maximumBlockSize, int numChannels);
    void setShift(Octaver::Shift shift) noexcept;
    void setShiftFromChoiceIndex(int choiceIndex) noexcept;
    void operator()(juce::AudioBuffer<float>& buffer);

private:
    enum class Algorithm
    {
        mcPherson,
        rubberBand
    };

    [[nodiscard]] static Algorithm getAlgorithmForShift(Octaver::Shift shift) noexcept;

    void updateAlgorithmPitch() noexcept;
    void resetActiveAlgorithm();

    Octaver::Shift shift { Octaver::Shift::oneDown };
    Algorithm activeAlgorithm { getAlgorithmForShift(shift) };

    class Impl;
    std::unique_ptr<Impl> impl;
};
}
