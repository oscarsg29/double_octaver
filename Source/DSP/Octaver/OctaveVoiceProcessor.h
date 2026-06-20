#pragma once

#include "Octaver.h"
#include "OctaverPitchShifter.h"
#include <JuceHeader.h>

namespace double_octaver::pitch
{
class OctaveVoiceProcessor
{
public:
    void prepare(double sampleRate, int maximumBlockSize, int numChannels);
    void update(float gainDb, bool bypassed, int shiftChoiceIndex);
    void process(const juce::AudioBuffer<float>& input, int numSamples);

    [[nodiscard]] bool isBypassed() const noexcept { return bypassed; }
    [[nodiscard]] juce::AudioBuffer<float>& getBuffer() noexcept { return buffer; }

private:
    void applyGain(int numSamples) noexcept;

    juce::AudioBuffer<float> buffer;
    OctaverPitchShifter pitchShifter;
    juce::SmoothedValue<float> gainLinear { 1.0f };
    bool bypassed = false;
};
}
