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
    shiftTransition.reset(sampleRate, 0.02);
    shiftTransition.setCurrentAndTargetValue(0.0f);
    lastOutputSamples.assign(numChannels, 0.0f);
    shiftTransitionOffsets.assign(numChannels, 0.0f);
    needsShiftTransition = false;
    updateAlgorithmPitch();
}

void OctaverPitchShifter::setShift(Octaver::Shift newShift) noexcept
{
    if (newShift == shift)
        return;

    shift = newShift;
    activeAlgorithm = getAlgorithmForShift(shift);
    updateAlgorithmPitch();
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

    if (activeAlgorithm == Algorithm::rubberBand)
        impl->rubberBandShifter.process(buffer);
    else
        impl->mcPhersonShifter.process(buffer);

    applyShiftTransition(buffer);
    storeLastOutputSamples(buffer);
}

OctaverPitchShifter::Algorithm OctaverPitchShifter::getAlgorithmForShift(Octaver::Shift shift) noexcept
{
    return Octaver::usesMcPhersonAlgorithm(shift) ? Algorithm::mcPherson
                                                 : Algorithm::rubberBand;
}

void OctaverPitchShifter::updateAlgorithmPitch() noexcept
{
    if (impl == nullptr)
        return;

    const auto semitones = Octaver::getShiftInSemitones(shift);

    impl->mcPhersonShifter.setSemitones(semitones);
    impl->rubberBandShifter.setSemitones(static_cast<float>(semitones));
}

void OctaverPitchShifter::applyShiftTransition(juce::AudioBuffer<float>& buffer) noexcept
{
    if (buffer.getNumSamples() == 0)
        return;

    if (needsShiftTransition)
    {
        const auto numChannels = std::min(buffer.getNumChannels(), static_cast<int>(shiftTransitionOffsets.size()));

        for (int channel = 0; channel < numChannels; ++channel)
            shiftTransitionOffsets[static_cast<size_t>(channel)] =
                lastOutputSamples[static_cast<size_t>(channel)] - buffer.getSample(channel, 0);

        needsShiftTransition = false;
    }

    if (! shiftTransition.isSmoothing())
        return;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto transition = shiftTransition.getNextValue();
        const auto numChannels = std::min(buffer.getNumChannels(), static_cast<int>(shiftTransitionOffsets.size()));

        for (int channel = 0; channel < numChannels; ++channel)
        {
            const auto offset = shiftTransitionOffsets[static_cast<size_t>(channel)] * transition;
            buffer.setSample(channel, sample, buffer.getSample(channel, sample) + offset);
        }
    }
}

void OctaverPitchShifter::storeLastOutputSamples(const juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumSamples() == 0)
        return;

    const auto numChannels = std::min(buffer.getNumChannels(), static_cast<int>(lastOutputSamples.size()));
    const auto lastSample = buffer.getNumSamples() - 1;

    for (int channel = 0; channel < numChannels; ++channel)
        lastOutputSamples[static_cast<size_t>(channel)] = buffer.getSample(channel, lastSample);
}
}
