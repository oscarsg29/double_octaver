#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParameterIds.h"
#include "DSP/Octaver/OctaveVoiceProcessor.h"

namespace parameters = double_octaver::parameters;
namespace pitch = double_octaver::pitch;

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
  voice1 = std::make_unique<pitch::OctaveVoiceProcessor>();
  voice2 = std::make_unique<pitch::OctaveVoiceProcessor>();
}

DoubleOctaverAudioProcessor::~DoubleOctaverAudioProcessor() = default;

juce::AudioProcessorValueTreeState& DoubleOctaverAudioProcessor::getValueTreeState() noexcept
{
  return apvts;
}

const juce::AudioProcessorValueTreeState& DoubleOctaverAudioProcessor::getValueTreeState() const noexcept
{
  return apvts;
}

juce::AudioProcessorValueTreeState::ParameterLayout
DoubleOctaverAudioProcessor::createParameters() {
  juce::AudioProcessorValueTreeState::ParameterLayout parameterLayout;

  constexpr auto gainKnobSkew = 1.5f;

  parameterLayout.add(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID(parameters::gain, 1), parameters::gain,
      juce::NormalisableRange<float>(Gain::MinGainDb, Gain::MaxGainDb, 0.1f, gainKnobSkew),
      Gain::UnityGainDb));

  parameterLayout.add(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID(parameters::dryWet, 1), parameters::dryWet, DryWet::MinDryWetPercent,
      DryWet::MaxDryWetPercent, DryWet::DefaultDryWetPercent));

  parameterLayout.add(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID(parameters::octaveGain1, 1), parameters::octaveGain1,
      juce::NormalisableRange<float>(Octaver::MinOctaveGainDb, Octaver::MaxOctaveGainDb, 0.1f, gainKnobSkew),
      Octaver::DefaultOctaveGainDb));

  parameterLayout.add(std::make_unique<juce::AudioParameterChoice>(
      juce::ParameterID(parameters::octaveShift1, 1), parameters::octaveShift1,
      juce::StringArray("-2 Oct", "-1 Oct", "+1 Oct", "+2 Oct"), 1));

  parameterLayout.add(std::make_unique<juce::AudioParameterBool>(
      juce::ParameterID(parameters::octaveBypass1, 1), parameters::octaveBypass1, false));

  parameterLayout.add(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID(parameters::octaveGain2, 1), parameters::octaveGain2,
      juce::NormalisableRange<float>(Octaver::MinOctaveGainDb, Octaver::MaxOctaveGainDb, 0.1f, gainKnobSkew),
      Octaver::DefaultOctaveGainDb));

  parameterLayout.add(std::make_unique<juce::AudioParameterChoice>(
      juce::ParameterID(parameters::octaveShift2, 1), parameters::octaveShift2,
      juce::StringArray("-2 Oct", "-1 Oct", "+1 Oct", "+2 Oct"), 2));

  parameterLayout.add(std::make_unique<juce::AudioParameterBool>(
      juce::ParameterID(parameters::octaveBypass2, 1), parameters::octaveBypass2, false));

  parameterLayout.add(std::make_unique<juce::AudioParameterBool>(
      juce::ParameterID(parameters::power, 1), parameters::power, true));

  return parameterLayout;
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
  const auto numChannels = getTotalNumOutputChannels();

  dryBuffer.setSize(numChannels, samplesPerBlock, false, false, true);
  voice1->prepare(sampleRate, samplesPerBlock, numChannels);
  voice2->prepare(sampleRate, samplesPerBlock, numChannels);
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

  jassert(buffer.getNumSamples() <= dryBuffer.getNumSamples());
  jassert(buffer.getNumChannels() == dryBuffer.getNumChannels());

  if (buffer.getNumSamples() > dryBuffer.getNumSamples() ||
      buffer.getNumChannels() != dryBuffer.getNumChannels())
    return;

  for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
    dryBuffer.copyFrom(channel, 0, buffer, channel, 0, buffer.getNumSamples());

  voice1->process(buffer, buffer.getNumSamples());
  voice2->process(buffer, buffer.getNumSamples());

  auto& octaveBuffer = voice1->getBuffer();
  auto& octaveBuffer2 = voice2->getBuffer();

  if (buffer.getNumChannels() == 1) {
    auto dryView = audio::MonoPolicy::makeView(dryBuffer);
    auto wetView = audio::MonoPolicy::makeView(octaveBuffer);
    auto monoView = audio::MonoPolicy::makeView(buffer);

    octaveBuffer.addFrom(0, 0, octaveBuffer2, 0, 0, buffer.getNumSamples());
    dsp::combineSamples(monoView, dryView, wetView, drywet);
    dsp::transformSamples(monoView, gain);

  } else if (buffer.getNumChannels() == 2) {
    auto dryView = audio::StereoPolicy::makeView(dryBuffer);
    auto wetView = audio::StereoPolicy::makeView(octaveBuffer);
    auto stereoView = audio::StereoPolicy::makeView(buffer);

    octaveBuffer.addFrom(0, 0, octaveBuffer2, 0, 0, buffer.getNumSamples());
    octaveBuffer.addFrom(1, 0, octaveBuffer2, 1, 0, buffer.getNumSamples());
    dsp::combineSamples(stereoView, dryView, wetView, drywet);
    dsp::transformSamples(stereoView, gain);
  }
}

void DoubleOctaverAudioProcessor::updateParameters() {
  gain.setGainDb(apvts.getRawParameterValue(parameters::gain)->load());
  voice1->update(apvts.getRawParameterValue(parameters::octaveGain1)->load(),
                 apvts.getRawParameterValue(parameters::octaveBypass1)->load() > 0.5f,
                 static_cast<int>(apvts.getRawParameterValue(parameters::octaveShift1)->load()));
  voice2->update(apvts.getRawParameterValue(parameters::octaveGain2)->load(),
                 apvts.getRawParameterValue(parameters::octaveBypass2)->load() > 0.5f,
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
