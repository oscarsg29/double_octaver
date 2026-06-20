#include "OctaverPitchShifter.h"
#include "../McPherson/McPhersonPitchShifter.h"
#include "../WangRubberband/WangRubberBandPitchShifter.h"

namespace double_octaver::pitch
{
class OctaverPitchShifter::Impl
{
public:
    McPhersonPitchShifter mcPhersonShifter;
    WangRubberBandPitchShifter rubberBandShifter;
    juce::AudioBuffer<float> mcPhersonBuffer;
    juce::AudioBuffer<float> rubberBandBuffer;
};

OctaverPitchShifter::OctaverPitchShifter() = default;
OctaverPitchShifter::~OctaverPitchShifter() = default;

void OctaverPitchShifter::prepare(double sampleRate, int maximumBlockSize, int numChannels)
{
    if (impl == nullptr)
        impl = std::make_unique<Impl>();

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(maximumBlockSize);
    spec.numChannels = static_cast<juce::uint32>(numChannels);

    impl->mcPhersonShifter.prepare(spec);
    impl->rubberBandShifter.prepare(spec);
    impl->mcPhersonBuffer.setSize(numChannels, maximumBlockSize);
    impl->rubberBandBuffer.setSize(numChannels, maximumBlockSize);
    shiftTransition.reset(sampleRate, 0.06);
    shiftTransition.setCurrentAndTargetValue(0.0f);
    needsShiftTransition = false;
    const auto semitones = Octaver::getShiftInSemitones(shift);
    setAlgorithmPitch(Algorithm::mcPherson, semitones);
    setAlgorithmPitch(Algorithm::rubberBand, semitones);
}

void OctaverPitchShifter::setShift(Octaver::Shift newShift) noexcept
{
    if (newShift == shift)
        return;

    previousAlgorithm = activeAlgorithm;
    shift = newShift;
    activeAlgorithm = getAlgorithmForShift(shift);
    updateActiveAlgorithmPitch();
    shiftTransition.setCurrentAndTargetValue(1.0f);
    shiftTransition.setTargetValue(0.0f);
    needsShiftTransition = true;
}

void OctaverPitchShifter::setShiftFromChoiceIndex(int choiceIndex) noexcept
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

void OctaverPitchShifter::operator()(juce::AudioBuffer<float>& buffer)
{
    jassert(impl != nullptr);

    if (impl == nullptr)
        return;

    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = buffer.getNumChannels();

    if (numSamples == 0)
        return;

    impl->mcPhersonBuffer.setSize(numChannels, numSamples, false, false, true);
    impl->rubberBandBuffer.setSize(numChannels, numSamples, false, false, true);

    for (int channel = 0; channel < numChannels; ++channel)
    {
        impl->mcPhersonBuffer.copyFrom(channel, 0, buffer, channel, 0, numSamples);
        impl->rubberBandBuffer.copyFrom(channel, 0, buffer, channel, 0, numSamples);
    }

    impl->mcPhersonShifter.process(impl->mcPhersonBuffer);
    impl->rubberBandShifter.process(impl->rubberBandBuffer);

    auto& activeBuffer = getAlgorithmBuffer(activeAlgorithm);
    for (int channel = 0; channel < numChannels; ++channel)
        buffer.copyFrom(channel, 0, activeBuffer, channel, 0, numSamples);

    applyShiftTransition(buffer);
}

OctaverPitchShifter::Algorithm OctaverPitchShifter::getAlgorithmForShift(Octaver::Shift shift) noexcept
{
    return Octaver::usesMcPhersonAlgorithm(shift) ? Algorithm::mcPherson
                                                 : Algorithm::rubberBand;
}

void OctaverPitchShifter::setAlgorithmPitch(Algorithm algorithm, int semitones) noexcept
{
    if (impl == nullptr)
        return;

    if (algorithm == Algorithm::rubberBand)
        impl->rubberBandShifter.setSemitones(static_cast<float>(semitones));
    else
        impl->mcPhersonShifter.setSemitones(semitones);
}

void OctaverPitchShifter::updateActiveAlgorithmPitch() noexcept
{
    setAlgorithmPitch(activeAlgorithm, Octaver::getShiftInSemitones(shift));
}

void OctaverPitchShifter::applyShiftTransition(juce::AudioBuffer<float>& buffer) noexcept
{
    if (buffer.getNumSamples() == 0)
        return;

    if (! shiftTransition.isSmoothing())
    {
        needsShiftTransition = false;
        return;
    }

    auto& previousBuffer = getAlgorithmBuffer(previousAlgorithm);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto transition = shiftTransition.getNextValue();

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            const auto previousSample = previousBuffer.getSample(channel, sample);
            const auto activeSample = buffer.getSample(channel, sample);
            buffer.setSample(channel, sample, previousSample * transition + activeSample * (1.0f - transition));
        }
    }

    needsShiftTransition = shiftTransition.isSmoothing();
}

juce::AudioBuffer<float>& OctaverPitchShifter::getAlgorithmBuffer(Algorithm algorithm) noexcept
{
    jassert(impl != nullptr);
    return algorithm == Algorithm::rubberBand ? impl->rubberBandBuffer
                                              : impl->mcPhersonBuffer;
}
}
