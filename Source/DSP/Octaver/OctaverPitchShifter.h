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
    void reset() noexcept;
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

    void setAlgorithmPitch(Algorithm algorithm, int semitones) noexcept;
    void updateActiveAlgorithmPitch() noexcept;
    void applyShiftTransition(juce::AudioBuffer<float>& buffer) noexcept;
    [[nodiscard]] juce::AudioBuffer<float>& getAlgorithmBuffer(Algorithm algorithm) noexcept;

    Octaver::Shift shift { Octaver::Shift::oneDown };
    Algorithm activeAlgorithm { getAlgorithmForShift(shift) };
    Algorithm previousAlgorithm { activeAlgorithm };
    juce::SmoothedValue<float> shiftTransition { 0.0f };
    bool needsShiftTransition = false;

    class Impl;
    std::unique_ptr<Impl> impl;
};
}
