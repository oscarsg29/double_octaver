/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Curso032026AudioProcessor::Curso032026AudioProcessor()
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
}

Curso032026AudioProcessor::~Curso032026AudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout
Curso032026AudioProcessor::createParameters() {
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
const juce::String Curso032026AudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool Curso032026AudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool Curso032026AudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool Curso032026AudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double Curso032026AudioProcessor::getTailLengthSeconds() const { return 0.0; }

int Curso032026AudioProcessor::getNumPrograms() {
  return 1; // NB: some hosts don't cope very well if you tell them there are 0
            // programs, so this should be at least 1, even if you're not really
            // implementing programs.
}

int Curso032026AudioProcessor::getCurrentProgram() { return 0; }

void Curso032026AudioProcessor::setCurrentProgram(int index) {}

const juce::String Curso032026AudioProcessor::getProgramName(int index) {
  return {};
}

void Curso032026AudioProcessor::changeProgramName(int index,
                                                  const juce::String &newName) {
}

//==============================================================================
void Curso032026AudioProcessor::prepareToPlay(double sampleRate,
                                              int samplesPerBlock) {
  octaver.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());

  // juce::dsp::ProcessSpec spec;
  // spec.sampleRate = sampleRate;
  // spec.maximumBlockSize = samplesPerBlock;
  // spec.numChannels = getTotalNumOutputChannels();
  //
  // lfo.prepare (sampleRate);
  // lpfBiquad.prepare (sampleRate);
  // filters.prepare (spec);
}

void Curso032026AudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Curso032026AudioProcessor::isBusesLayoutSupported(
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

void Curso032026AudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                             juce::MidiBuffer &midiMessages) {
  juce::ignoreUnused(midiMessages);

  updateParameters();

  if (buffer.getNumChannels() == 0)
    return;

  dryBuffer.makeCopyOf(buffer);
  octaveBuffer.makeCopyOf(buffer);

  octaver.process(octaveBuffer);


  if (buffer.getNumChannels() == 1) {
    auto dryView = audio::MonoPolicy::makeView(dryBuffer);
    auto wetView = audio::MonoPolicy::makeView(octaveBuffer);
    auto monoView = audio::MonoPolicy::makeView(buffer);

    dsp::combineSamples(monoView, dryView, wetView, drywet);
    dsp::transformSamples(monoView, gain);

  } else if (buffer.getNumChannels() == 2) {
    auto dryView = audio::StereoPolicy::makeView(dryBuffer);
    auto wetView = audio::StereoPolicy::makeView(octaveBuffer);
    auto stereoView = audio::StereoPolicy::makeView(buffer);

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

void Curso032026AudioProcessor::updateParameters() {
  gain.setGainDb(apvts.getRawParameterValue("Gain")->load());
  octaver.setOctaveGainDb(apvts.getRawParameterValue("OctaveGain")->load());
  octaver.setShiftFromChoiceIndex(
      static_cast<int>(apvts.getRawParameterValue("OctaveShift")->load()));
  // panning.setPanValue (apvts.getRawParameterValue("Panning")->load());
  // lfo.setFrequencyValue (apvts.getRawParameterValue("LFO")->load());
  drywet.setDryWetPercent(apvts.getRawParameterValue("DryWet")->load());
}

//==============================================================================
bool Curso032026AudioProcessor::hasEditor() const {
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *Curso032026AudioProcessor::createEditor() {
  //    return new Curso032026AudioProcessorEditor (*this);
  return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void Curso032026AudioProcessor::getStateInformation(
    juce::MemoryBlock &destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
}

void Curso032026AudioProcessor::setStateInformation(const void *data,
                                                    int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new Curso032026AudioProcessor();
}
