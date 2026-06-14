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
    juce::AudioBuffer<float> buffer;
    Octaver gain;
    OctaverPitchShifter pitchShifter;
    bool bypassed = false;
};
}
