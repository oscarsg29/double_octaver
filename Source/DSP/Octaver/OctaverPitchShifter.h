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
    void applyShiftTransition(juce::AudioBuffer<float>& buffer) noexcept;
    void storeLastOutputSamples(const juce::AudioBuffer<float>& buffer);

    Octaver::Shift shift { Octaver::Shift::oneDown };
    Algorithm activeAlgorithm { getAlgorithmForShift(shift) };
    juce::SmoothedValue<float> shiftTransition { 0.0f };
    std::vector<float> lastOutputSamples;
    std::vector<float> shiftTransitionOffsets;
    bool needsShiftTransition = false;

    class Impl;
    std::unique_ptr<Impl> impl;
};
}
