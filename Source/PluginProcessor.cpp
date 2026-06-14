#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParameterIds.h"

#include "DSP/McPherson/McPhersonPitchShifter.h"
#include "DSP/WangRubberband/WangRubberBandPitchShifter.h"

namespace parameters = double_octaver::parameters;

class DoubleOctaverAudioProcessor::OctaverPitchShifter {
  public:
    void prepare(double sampleRate, int maximumBlockSize, int numChannels)
    {
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32>(maximumBlockSize);
        spec.numChannels = static_cast<juce::uint32>(numChannels);

        mcPhersonShifter_.prepare(spec);
        rubberBandShifter_.prepare(spec);
        updateAlgorithmPitch();
    }

    void setShift(Octaver::Shift shift) noexcept
    {
        if (shift == shift_)
            return;

        shift_ = shift;
        const auto nextAlgorithm = getAlgorithmForShift(shift_);

        activeAlgorithm_ = nextAlgorithm;
        updateAlgorithmPitch();
        resetActiveAlgorithm();
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

    void operator()(juce::AudioBuffer<float>& buffer)
    {
        if (activeAlgorithm_ == Algorithm::rubberBand)
            rubberBandShifter_.process(buffer);
        else
            mcPhersonShifter_.process(buffer);
    }

  private:
    enum class Algorithm {
        mcPherson,
        rubberBand
    };

    [[nodiscard]] static Algorithm getAlgorithmForShift(Octaver::Shift shift) noexcept
    {
        return Octaver::usesMcPhersonAlgorithm(shift)
                   ? Algorithm::mcPherson
                   : Algorithm::rubberBand;
    }

    void updateAlgorithmPitch() noexcept
    {
        const auto semitones = Octaver::getShiftInSemitones(shift_);

        mcPhersonShifter_.setSemitones(semitones);
        rubberBandShifter_.setSemitones(static_cast<float>(semitones));
    }

    void resetActiveAlgorithm()
    {
        if (activeAlgorithm_ == Algorithm::rubberBand)
            rubberBandShifter_.reset();
        else
            mcPhersonShifter_.reset();
    }

    Octaver::Shift shift_{Octaver::Shift::oneDown};
    Algorithm activeAlgorithm_{getAlgorithmForShift(shift_)};
    McPhersonPitchShifter mcPhersonShifter_;
    WangRubberBandPitchShifter rubberBandShifter_;
};

DoubleOctaverAudioProcessor::DoubleOctaverAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
              ),
      apvts(*this, nullptr, "Parameters", createParameters())
#endif
{
  octaverPitchShifter = std::make_unique<OctaverPitchShifter>();
  octaverPitchShifter2 = std::make_unique<OctaverPitchShifter>();
}

DoubleOctaverAudioProcessor::~DoubleOctaverAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout
DoubleOctaverAudioProcessor::createParameters() {
  juce::AudioProcessorValueTreeState::ParameterLayout parameters;

  constexpr auto gainKnobSkew = 1.5f;

  parameters.add(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID(parameters::gain, 1), parameters::gain,
      juce::NormalisableRange<float>(Gain::MinGainDb, Gain::MaxGainDb, 0.1f, gainKnobSkew),
      Gain::UnityGainDb));

  parameters.add(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID(parameters::dryWet, 1), parameters::dryWet, DryWet::MinDryWetPercent,
      DryWet::MaxDryWetPercent, DryWet::DefaultDryWetPercent));

  parameters.add(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID(parameters::octaveGain1, 1), parameters::octaveGain1,
      juce::NormalisableRange<float>(Octaver::MinOctaveGainDb, Octaver::MaxOctaveGainDb, 0.1f, gainKnobSkew),
      Octaver::DefaultOctaveGainDb));

  parameters.add(std::make_unique<juce::AudioParameterChoice>(
      juce::ParameterID(parameters::octaveShift1, 1), parameters::octaveShift1,
      juce::StringArray("-2 Oct", "-1 Oct", "+1 Oct", "+2 Oct"), 1));

  parameters.add(std::make_unique<juce::AudioParameterBool>(
      juce::ParameterID(parameters::octaveBypass1, 1), parameters::octaveBypass1, false));

  parameters.add(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID(parameters::octaveGain2, 1), parameters::octaveGain2,
      juce::NormalisableRange<float>(Octaver::MinOctaveGainDb, Octaver::MaxOctaveGainDb, 0.1f, gainKnobSkew),
      Octaver::DefaultOctaveGainDb));

  parameters.add(std::make_unique<juce::AudioParameterChoice>(
      juce::ParameterID(parameters::octaveShift2, 1), parameters::octaveShift2,
      juce::StringArray("-2 Oct", "-1 Oct", "+1 Oct", "+2 Oct"), 2));

  parameters.add(std::make_unique<juce::AudioParameterBool>(
      juce::ParameterID(parameters::octaveBypass2, 1), parameters::octaveBypass2, false));

  parameters.add(std::make_unique<juce::AudioParameterBool>(
      juce::ParameterID(parameters::power, 1), parameters::power, true));

  return parameters;
}

const juce::String DoubleOctaverAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool DoubleOctaverAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool DoubleOctaverAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool DoubleOctaverAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double DoubleOctaverAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int DoubleOctaverAudioProcessor::getNumPrograms() {
  return 1;
}

int DoubleOctaverAudioProcessor::getCurrentProgram() { return 0; }

void DoubleOctaverAudioProcessor::setCurrentProgram(int) {}

const juce::String DoubleOctaverAudioProcessor::getProgramName(int) {
  return {};
}

void DoubleOctaverAudioProcessor::changeProgramName(int,
                                                  const juce::String &) {
}

void DoubleOctaverAudioProcessor::prepareToPlay(double sampleRate,
                                              int samplesPerBlock) {
  octaverPitchShifter->prepare(sampleRate, samplesPerBlock,
                               getTotalNumOutputChannels());
  octaverPitchShifter2->prepare(sampleRate, samplesPerBlock,
                                getTotalNumOutputChannels());
}

void DoubleOctaverAudioProcessor::releaseResources() {
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DoubleOctaverAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}
#endif

void DoubleOctaverAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                             juce::MidiBuffer &midiMessages) {
  juce::ignoreUnused(midiMessages);

  if (apvts.getRawParameterValue(parameters::power)->load() < 0.5f)
    return;

  updateParameters();

  if (buffer.getNumChannels() == 0)
    return;

  dryBuffer.makeCopyOf(buffer);
  octaveBuffer.makeCopyOf(buffer);
  octaveBuffer2.makeCopyOf(buffer);

  if (octaveBypassed)
    octaveBuffer.clear();
  else
    (*octaverPitchShifter)(octaveBuffer);

  if (octave2Bypassed)
    octaveBuffer2.clear();
  else
    (*octaverPitchShifter2)(octaveBuffer2);

  if (buffer.getNumChannels() == 1) {
    auto dryView = audio::MonoPolicy::makeView(dryBuffer);
    auto wetView = audio::MonoPolicy::makeView(octaveBuffer);
    auto wetView2 = audio::MonoPolicy::makeView(octaveBuffer2);
    auto monoView = audio::MonoPolicy::makeView(buffer);

    if (! octaveBypassed)
      dsp::transformSamples(wetView, octaver);

    if (! octave2Bypassed)
      dsp::transformSamples(wetView2, octaver2);

    octaveBuffer.addFrom(0, 0, octaveBuffer2, 0, 0, buffer.getNumSamples());
    dsp::combineSamples(monoView, dryView, wetView, drywet);
    dsp::transformSamples(monoView, gain);

  } else if (buffer.getNumChannels() == 2) {
    auto dryView = audio::StereoPolicy::makeView(dryBuffer);
    auto wetView = audio::StereoPolicy::makeView(octaveBuffer);
    auto wetView2 = audio::StereoPolicy::makeView(octaveBuffer2);
    auto stereoView = audio::StereoPolicy::makeView(buffer);

    if (! octaveBypassed)
      dsp::transformSamples(wetView, octaver);

    if (! octave2Bypassed)
      dsp::transformSamples(wetView2, octaver2);

    octaveBuffer.addFrom(0, 0, octaveBuffer2, 0, 0, buffer.getNumSamples());
    octaveBuffer.addFrom(1, 0, octaveBuffer2, 1, 0, buffer.getNumSamples());
    dsp::combineSamples(stereoView, dryView, wetView, drywet);
    dsp::transformSamples(stereoView, gain);
  }
}

void DoubleOctaverAudioProcessor::updateParameters() {
  gain.setGainDb(apvts.getRawParameterValue(parameters::gain)->load());
  octaver.setOctaveGainDb(apvts.getRawParameterValue(parameters::octaveGain1)->load());
  octaver2.setOctaveGainDb(apvts.getRawParameterValue(parameters::octaveGain2)->load());
  octaveBypassed = apvts.getRawParameterValue(parameters::octaveBypass1)->load() > 0.5f;
  octave2Bypassed = apvts.getRawParameterValue(parameters::octaveBypass2)->load() > 0.5f;
  octaverPitchShifter->setShiftFromChoiceIndex(
      static_cast<int>(apvts.getRawParameterValue(parameters::octaveShift1)->load()));
  octaverPitchShifter2->setShiftFromChoiceIndex(
      static_cast<int>(apvts.getRawParameterValue(parameters::octaveShift2)->load()));
  drywet.setDryWetPercent(apvts.getRawParameterValue(parameters::dryWet)->load());
}

bool DoubleOctaverAudioProcessor::hasEditor() const {
  return true;
}

juce::AudioProcessorEditor *DoubleOctaverAudioProcessor::createEditor() {
  return new DoubleOctaverAudioProcessorEditor(*this);
}

void DoubleOctaverAudioProcessor::getStateInformation(
    juce::MemoryBlock &destData) {
  if (auto state = apvts.copyState().createXml())
    copyXmlToBinary(*state, destData);
}

void DoubleOctaverAudioProcessor::setStateInformation(const void *data,
                                                    int sizeInBytes) {
  if (auto state = getXmlFromBinary(data, sizeInBytes))
  {
    if (state->hasTagName(apvts.state.getType()))
      apvts.replaceState(juce::ValueTree::fromXml(*state));
  }
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new DoubleOctaverAudioProcessor();
}
