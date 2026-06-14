#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

namespace double_octaver::gui
{
class BypassButtonLookAndFeel;
class PowerButtonLookAndFeel;
class RotaryLookAndFeel;
class VoiceControls;
}

class DoubleOctaverAudioProcessorEditor  : public juce::AudioProcessorEditor
                                          , private juce::Timer
{
public:
    DoubleOctaverAudioProcessorEditor (DoubleOctaverAudioProcessor&);
    ~DoubleOctaverAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    void timerCallback() override;
    float getContentScale() const noexcept;
    void configureSlider(juce::Slider& slider, const juce::String& suffix, juce::Colour thumbColour);
    void drawPanel(juce::Graphics& g);
    void drawHeader(juce::Graphics& g, bool powerOn);
    void drawSectionLabels(juce::Graphics& g);
    void drawFooter(juce::Graphics& g, bool powerOn, bool voice1On, bool voice2On);
    void drawStatusDot(juce::Graphics& g, juce::Point<float> centre,
                       juce::Colour colour, bool active, const juce::String& label);

    DoubleOctaverAudioProcessor& audioProcessor;

    std::unique_ptr<double_octaver::gui::RotaryLookAndFeel> rotaryLookAndFeel;
    std::unique_ptr<double_octaver::gui::PowerButtonLookAndFeel> powerButtonLookAndFeel;
    std::unique_ptr<double_octaver::gui::BypassButtonLookAndFeel> bypassButtonLookAndFeel;

    juce::Slider dryWetSlider;
    juce::Slider gainSlider;

    std::unique_ptr<double_octaver::gui::VoiceControls> voice1Controls;
    std::unique_ptr<double_octaver::gui::VoiceControls> voice2Controls;
    juce::ToggleButton powerButton;

    std::unique_ptr<SliderAttachment> dryWetAttachment;
    std::unique_ptr<SliderAttachment> gainAttachment;
    std::unique_ptr<ButtonAttachment> powerAttachment;

    float controlsAlpha = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DoubleOctaverAudioProcessorEditor)
};
