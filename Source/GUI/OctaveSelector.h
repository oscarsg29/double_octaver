#pragma once

#include <JuceHeader.h>

namespace double_octaver::gui
{
class OctaveSelector : public juce::Component,
                       private juce::Timer
{
public:
    OctaveSelector(juce::AudioProcessorValueTreeState& state, const juce::String& parameterId,
                   juce::String selectorName, juce::Colour accentColour);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    juce::Rectangle<int> getSelectorBounds() const;
    int getParameterIndex() const;
    void setParameterIndex(int index);

    juce::RangedAudioParameter* parameter = nullptr;
    juce::String name;
    juce::Colour colour;
    std::array<juce::TextButton, 4> buttons;
    std::array<juce::String, 4> options { "-2", "-1", "+1", "+2" };
    int selectedIndex = 0;
    float indicatorPosition = 0.0f;
};
}
