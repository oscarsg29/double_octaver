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
    updateAlgorithmPitch();
}

void OctaverPitchShifter::setShift(Octaver::Shift newShift) noexcept
{
    if (newShift == shift)
        return;

    shift = newShift;
    activeAlgorithm = getAlgorithmForShift(shift);
    updateAlgorithmPitch();
    resetActiveAlgorithm();
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

void OctaverPitchShifter::resetActiveAlgorithm()
{
    if (impl == nullptr)
        return;

    if (activeAlgorithm == Algorithm::rubberBand)
        impl->rubberBandShifter.reset();
    else
        impl->mcPhersonShifter.reset();
}
}
