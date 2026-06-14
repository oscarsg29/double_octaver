#pragma once

#include <JuceHeader.h>
#include "Theme.h"

namespace double_octaver::gui
{
inline void configureRotarySlider(juce::Slider& slider, juce::LookAndFeel_V4& lookAndFeel,
                                  const juce::String& suffix, juce::Colour thumbColour)
{
    slider.setLookAndFeel(&lookAndFeel);
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 16);
    slider.setRotaryParameters(juce::degreesToRadians(215.0f),
                               juce::degreesToRadians(505.0f), true);
    slider.setTextValueSuffix(suffix);
    slider.setColour(juce::Slider::thumbColourId, thumbColour);
    slider.setColour(juce::Slider::textBoxTextColourId, theme::cream.withAlpha(0.8f));
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
}
}
