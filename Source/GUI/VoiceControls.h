#pragma once

#include <JuceHeader.h>
#include "LookAndFeels.h"
#include "OctaveSelector.h"

namespace double_octaver::gui
{
class VoiceControls : public juce::Component,
                      private juce::Timer
{
public:
    VoiceControls(juce::AudioProcessorValueTreeState& state,
                  const juce::String& gainParameterId,
                  const juce::String& shiftParameterId,
                  const juce::String& bypassParameterId,
                  juce::String selectorName,
                  juce::Colour voiceColour,
                  RotaryLookAndFeel& rotaryLookAndFeel,
                  BypassButtonLookAndFeel& bypassButtonLookAndFeel);

    ~VoiceControls() override;

    void resized() override;
    void setPowerAlpha(float alpha);
    bool isActive() const;

private:
    void timerCallback() override;
    void configureGainSlider(RotaryLookAndFeel& rotaryLookAndFeel, juce::Colour voiceColour);

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::atomic<float>* bypassParameter = nullptr;
    juce::Slider gainSlider;
    std::unique_ptr<OctaveSelector> selector;
    juce::ToggleButton bypassButton;
    std::unique_ptr<SliderAttachment> gainAttachment;
    std::unique_ptr<ButtonAttachment> bypassAttachment;
    float powerAlpha = 1.0f;
};
}
