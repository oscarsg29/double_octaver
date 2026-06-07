/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "DSP/McPherson/McPhersonPitchShifter.h"
#include "DSP/WangRubberband/WangRubberBandPitchShifter.h"

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

    [[nodiscard]] Octaver::Shift getShift() const noexcept { return shift_; }

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

//==============================================================================
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

  parameters.add(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID("Gain", 1), "Gain", Gain::MinGainDb, Gain::MaxGainDb,
      Gain::UnityGainDb));

  /*parameters.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Panning",
  1), "Panning", -100.0f, 100.0f, 0.0f));

  parameters.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("LFO",
  1), "LFO", 0.01f, 20.0f, 20.0f));*/

  parameters.add(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID("DryWet", 1), "DryWet", DryWet::MinDryWetPercent,
      DryWet::MaxDryWetPercent, DryWet::DefaultDryWetPercent));

  parameters.add(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID("OctaveGain", 1), "OctaveGain",
      Octaver::MinOctaveGainDb, Octaver::MaxOctaveGainDb,
      Octaver::DefaultOctaveGainDb));

  parameters.add(std::make_unique<juce::AudioParameterChoice>(
      juce::ParameterID("OctaveShift", 1), "OctaveShift",
      juce::StringArray("-2 Oct", "-1 Oct", "+1 Oct", "+2 Oct"), 1));

  parameters.add(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID("OctaveGain2", 1), "OctaveGain2",
      Octaver::MinOctaveGainDb, Octaver::MaxOctaveGainDb,
      Octaver::MinOctaveGainDb));

  parameters.add(std::make_unique<juce::AudioParameterChoice>(
      juce::ParameterID("OctaveShift2", 1), "OctaveShift2",
      juce::StringArray("-2 Oct", "-1 Oct", "+1 Oct", "+2 Oct"), 0));

  parameters.add(std::make_unique<juce::AudioParameterBool>(
      juce::ParameterID("Power", 1), "Power", true));

  /* parameters.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID("Int",
   1), "Int", 0, 100, 80));

   parameters.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID
   ("Bool", 1), "Bool", true));

   parameters.add (std::make_unique<juce::AudioParameterChoice>
   (juce::ParameterID ("Choice", 1), "Choice", juce::StringArray ("Sine", "Saw",
                                                                                    "Square",
                                                                                    "Triangle"),
                                                                 0));
    */

  return parameters;
}

//==============================================================================
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
  return 1; // NB: some hosts don't cope very well if you tell them there are 0
            // programs, so this should be at least 1, even if you're not really
            // implementing programs.
}

int DoubleOctaverAudioProcessor::getCurrentProgram() { return 0; }

void DoubleOctaverAudioProcessor::setCurrentProgram(int index) {}

const juce::String DoubleOctaverAudioProcessor::getProgramName(int index) {
  return {};
}

void DoubleOctaverAudioProcessor::changeProgramName(int index,
                                                  const juce::String &newName) {
}

//==============================================================================
void DoubleOctaverAudioProcessor::prepareToPlay(double sampleRate,
                                              int samplesPerBlock) {
  octaverPitchShifter->prepare(sampleRate, samplesPerBlock,
                               getTotalNumOutputChannels());
  octaverPitchShifter2->prepare(sampleRate, samplesPerBlock,
                                getTotalNumOutputChannels());

  // juce::dsp::ProcessSpec spec;
  // spec.sampleRate = sampleRate;
  // spec.maximumBlockSize = samplesPerBlock;
  // spec.numChannels = getTotalNumOutputChannels();
  //
  // lfo.prepare (sampleRate);
  // lpfBiquad.prepare (sampleRate);
  // filters.prepare (spec);
}

void DoubleOctaverAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DoubleOctaverAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

    // This checks if the input layout matches the output layout
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

  if (apvts.getRawParameterValue("Power")->load() < 0.5f)
    return;

  updateParameters();

  if (buffer.getNumChannels() == 0)
    return;

  dryBuffer.makeCopyOf(buffer);
  octaveBuffer.makeCopyOf(buffer);
  octaveBuffer2.makeCopyOf(buffer);


  (*octaverPitchShifter)(octaveBuffer);
  (*octaverPitchShifter2)(octaveBuffer2);

  if (buffer.getNumChannels() == 1) {
    auto dryView = audio::MonoPolicy::makeView(dryBuffer);
    auto wetView = audio::MonoPolicy::makeView(octaveBuffer);
    auto wetView2 = audio::MonoPolicy::makeView(octaveBuffer2);
    auto monoView = audio::MonoPolicy::makeView(buffer);

    dsp::transformSamples(wetView, octaver);
    dsp::transformSamples(wetView2, octaver2);
    octaveBuffer.addFrom(0, 0, octaveBuffer2, 0, 0, buffer.getNumSamples());
    dsp::combineSamples(monoView, dryView, wetView, drywet);
    dsp::transformSamples(monoView, gain);

  } else if (buffer.getNumChannels() == 2) {
    auto dryView = audio::StereoPolicy::makeView(dryBuffer);
    auto wetView = audio::StereoPolicy::makeView(octaveBuffer);
    auto wetView2 = audio::StereoPolicy::makeView(octaveBuffer2);
    auto stereoView = audio::StereoPolicy::makeView(buffer);

    dsp::transformSamples(wetView, octaver);
    dsp::transformSamples(wetView2, octaver2);
    octaveBuffer.addFrom(0, 0, octaveBuffer2, 0, 0, buffer.getNumSamples());
    octaveBuffer.addFrom(1, 0, octaveBuffer2, 1, 0, buffer.getNumSamples());
    dsp::combineSamples(stereoView, dryView, wetView, drywet);
    dsp::transformSamples(stereoView, gain);
  }

  // panning.process (buffer);
  // lfo.process (buffer);
  // lpfBiquad.process (buffer);
  // filters.processLowpass (buffer);
  // filters.processHighpass (buffer);
  // filters.processBandpass (buffer);
}

void DoubleOctaverAudioProcessor::updateParameters() {
  gain.setGainDb(apvts.getRawParameterValue("Gain")->load());
  octaver.setOctaveGainDb(apvts.getRawParameterValue("OctaveGain")->load());
  octaver2.setOctaveGainDb(apvts.getRawParameterValue("OctaveGain2")->load());
  octaverPitchShifter->setShiftFromChoiceIndex(
      static_cast<int>(apvts.getRawParameterValue("OctaveShift")->load()));
  octaverPitchShifter2->setShiftFromChoiceIndex(
      static_cast<int>(apvts.getRawParameterValue("OctaveShift2")->load()));
  // panning.setPanValue (apvts.getRawParameterValue("Panning")->load());
  // lfo.setFrequencyValue (apvts.getRawParameterValue("LFO")->load());
  drywet.setDryWetPercent(apvts.getRawParameterValue("DryWet")->load());
}

//==============================================================================
bool DoubleOctaverAudioProcessor::hasEditor() const {
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *DoubleOctaverAudioProcessor::createEditor() {
  return new DoubleOctaverAudioProcessorEditor(*this);
}

//==============================================================================
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

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new DoubleOctaverAudioProcessor();
}
