/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class DoubleOctaverAudioProcessorEditor  : public juce::AudioProcessorEditor
                                          , private juce::Timer
{
public:
    DoubleOctaverAudioProcessorEditor (DoubleOctaverAudioProcessor&);
    ~DoubleOctaverAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class OctaveSelector;
    class PowerButtonLookAndFeel;
    class RotaryLookAndFeel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    void timerCallback() override;
    void configureSlider(juce::Slider& slider, const juce::String& suffix);
    void drawStatusDot(juce::Graphics& g, juce::Point<float> centre,
                       juce::Colour colour, bool active, const juce::String& label);

    DoubleOctaverAudioProcessor& audioProcessor;

    RotaryLookAndFeel* rotaryLookAndFeel = nullptr;
    PowerButtonLookAndFeel* powerButtonLookAndFeel = nullptr;

    juce::Slider octaveGainSlider;
    juce::Slider octaveGain2Slider;
    juce::Slider dryWetSlider;
    juce::Slider gainSlider;

    OctaveSelector* octaveSelector = nullptr;
    OctaveSelector* octaveSelector2 = nullptr;
    juce::ToggleButton powerButton;

    std::unique_ptr<SliderAttachment> octaveGainAttachment;
    std::unique_ptr<SliderAttachment> octaveGain2Attachment;
    std::unique_ptr<SliderAttachment> dryWetAttachment;
    std::unique_ptr<SliderAttachment> gainAttachment;
    std::unique_ptr<ButtonAttachment> powerAttachment;

    float controlsAlpha = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DoubleOctaverAudioProcessorEditor)
};
