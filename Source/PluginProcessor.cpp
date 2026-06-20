#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParameterIds.h"
#include "DSP/Octaver/OctaveVoiceProcessor.h"

namespace parameters = double_octaver::parameters;
namespace pitch = double_octaver::pitch;

namespace
{
juce::AudioParameterFloatAttributes makeGainParameterAttributes(float minimumGainDb)
{
  return juce::AudioParameterFloatAttributes()
      .withStringFromValueFunction([minimumGainDb](float value, int maximumStringLength)
      {
        juce::String text = value <= minimumGainDb ? "-inf" : juce::String(value, 1);
        return maximumStringLength > 0 ? text.substring(0, maximumStringLength) : text;
      })
      .withValueFromStringFunction([](const juce::String& text)
      {
        return text.getFloatValue();
      });
}
}

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
      Gain::UnityGainDb,
      makeGainParameterAttributes(Gain::MinGainDb)));

  parameterLayout.add(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID(parameters::dryWet, 1), parameters::dryWet, DryWet::MinDryWetPercent,
      DryWet::MaxDryWetPercent, DryWet::DefaultDryWetPercent));

  parameterLayout.add(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID(parameters::octaveGain1, 1), parameters::octaveGain1,
      juce::NormalisableRange<float>(Octaver::MinOctaveGainDb, Octaver::MaxOctaveGainDb, 0.1f, gainKnobSkew),
      Octaver::DefaultOctaveGainDb,
      makeGainParameterAttributes(Octaver::MinOctaveGainDb)));

  parameterLayout.add(std::make_unique<juce::AudioParameterChoice>(
      juce::ParameterID(parameters::octaveShift1, 1), parameters::octaveShift1,
      juce::StringArray("-2 Oct", "-1 Oct", "+1 Oct", "+2 Oct"), 1));

  parameterLayout.add(std::make_unique<juce::AudioParameterBool>(
      juce::ParameterID(parameters::octaveBypass1, 1), parameters::octaveBypass1, false));

  parameterLayout.add(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID(parameters::octaveGain2, 1), parameters::octaveGain2,
      juce::NormalisableRange<float>(Octaver::MinOctaveGainDb, Octaver::MaxOctaveGainDb, 0.1f, gainKnobSkew),
      Octaver::DefaultOctaveGainDb,
      makeGainParameterAttributes(Octaver::MinOctaveGainDb)));

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
  outputGainLinear.reset(sampleRate, 0.02);
  outputGainLinear.setCurrentAndTargetValue(1.0f);
  wetMix.reset(sampleRate, 0.02);
  wetMix.setCurrentAndTargetValue(DryWet::DefaultDryWetPercent / DryWet::MaxDryWetPercent);
  powerMix.reset(sampleRate, 0.03);
  powerMix.setCurrentAndTargetValue(1.0f);
  transportWetFade.reset(sampleRate, 0.05);
  transportWetFade.setCurrentAndTargetValue(1.0f);
  lastPlayheadSamplePosition.reset();
  wasPlaying = false;
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

  updateParameters();
  updateTransportDiscontinuity(buffer.getNumSamples());

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
    octaveBuffer.addFrom(0, 0, octaveBuffer2, 0, 0, buffer.getNumSamples());
    combineAndApplyOutputGain(buffer, dryBuffer, octaveBuffer, buffer.getNumSamples());

  } else if (buffer.getNumChannels() == 2) {
    octaveBuffer.addFrom(0, 0, octaveBuffer2, 0, 0, buffer.getNumSamples());
    octaveBuffer.addFrom(1, 0, octaveBuffer2, 1, 0, buffer.getNumSamples());
    combineAndApplyOutputGain(buffer, dryBuffer, octaveBuffer, buffer.getNumSamples());
  }
}

void DoubleOctaverAudioProcessor::updateParameters() {
  const auto outputGainDb = std::clamp(apvts.getRawParameterValue(parameters::gain)->load(),
                                       Gain::MinGainDb,
                                       Gain::MaxGainDb);
  outputGainLinear.setTargetValue(std::pow(10.0f, outputGainDb / 20.0f));
  voice1->update(apvts.getRawParameterValue(parameters::octaveGain1)->load(),
                 apvts.getRawParameterValue(parameters::octaveBypass1)->load() > 0.5f,
                 static_cast<int>(apvts.getRawParameterValue(parameters::octaveShift1)->load()));
  voice2->update(apvts.getRawParameterValue(parameters::octaveGain2)->load(),
                 apvts.getRawParameterValue(parameters::octaveBypass2)->load() > 0.5f,
                 static_cast<int>(apvts.getRawParameterValue(parameters::octaveShift2)->load()));
  const auto dryWetPercent = std::clamp(apvts.getRawParameterValue(parameters::dryWet)->load(),
                                        DryWet::MinDryWetPercent,
                                        DryWet::MaxDryWetPercent);
  wetMix.setTargetValue(dryWetPercent / DryWet::MaxDryWetPercent);

  powerMix.setTargetValue(apvts.getRawParameterValue(parameters::power)->load() >= 0.5f ? 1.0f : 0.0f);
}

void DoubleOctaverAudioProcessor::updateTransportDiscontinuity(int numSamples)
{
  auto* playHead = getPlayHead();
  if (playHead == nullptr)
  {
    lastPlayheadSamplePosition.reset();
    wasPlaying = false;
    return;
  }

  const auto position = playHead->getPosition();
  const auto timeInSamples = position->getTimeInSamples();
  const auto isPlaying = position->getIsPlaying();

  if (! timeInSamples.hasValue())
  {
    lastPlayheadSamplePosition.reset();
    wasPlaying = isPlaying;
    return;
  }

  const auto currentSample = *timeInSamples;
  auto shouldReset = false;

  if (lastPlayheadSamplePosition.hasValue())
  {
    const auto expectedNextSample = *lastPlayheadSamplePosition + numSamples;
    constexpr int64_t toleranceSamples = 4096;
    shouldReset = std::llabs(currentSample - expectedNextSample) > toleranceSamples;
  }

  if (isPlaying && ! wasPlaying)
    shouldReset = true;

  if (shouldReset)
    resetEffectPathForTransportJump();

  lastPlayheadSamplePosition = currentSample;
  wasPlaying = isPlaying;
}

void DoubleOctaverAudioProcessor::resetEffectPathForTransportJump() noexcept
{
  voice1->reset();
  voice2->reset();
  transportWetFade.setCurrentAndTargetValue(0.0f);
  transportWetFade.setTargetValue(1.0f);
}

void DoubleOctaverAudioProcessor::combineAndApplyOutputGain(juce::AudioBuffer<float>& output,
                                                            const juce::AudioBuffer<float>& dry,
                                                            const juce::AudioBuffer<float>& wet,
                                                            int numSamples) noexcept
{
  const auto numChannels = std::min({ output.getNumChannels(), dry.getNumChannels(), wet.getNumChannels() });

  for (int sample = 0; sample < numSamples; ++sample)
  {
    const auto wetRatio = wetMix.getNextValue() * transportWetFade.getNextValue();
    const auto dryRatio = 1.0f - wetRatio;
    const auto gain = outputGainLinear.getNextValue();
    const auto power = powerMix.getNextValue();

    for (int channel = 0; channel < numChannels; ++channel)
    {
      const auto mixed = dry.getSample(channel, sample) * dryRatio
                       + wet.getSample(channel, sample) * wetRatio;
      const auto processed = mixed * gain;
      output.setSample(channel, sample, dry.getSample(channel, sample) * (1.0f - power)
                                      + processed * power);
    }
  }
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
