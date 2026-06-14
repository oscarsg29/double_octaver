#pragma once

#include <JuceHeader.h>

namespace double_octaver::gui::theme
{
inline const auto accent = juce::Colour::fromRGB(0xe8, 0x73, 0x0a);
inline const auto blue = juce::Colour::fromRGB(0x4a, 0x9e, 0xff);
inline const auto cream = juce::Colour::fromRGB(0xe8, 0xe4, 0xd8);
inline const auto green = juce::Colour::fromRGB(0xa0, 0xc8, 0x78);

inline juce::Colour panelTop() { return juce::Colour::fromRGB(0x25, 0x25, 0x28); }
inline juce::Colour panelBottom() { return juce::Colour::fromRGB(0x14, 0x14, 0x18); }
inline juce::Colour stripColour() { return juce::Colour::fromRGB(0x11, 0x11, 0x14); }

inline juce::Font monoFont(float height)
{
    return juce::FontOptions("Menlo", height, juce::Font::plain);
}

inline juce::Font titleFont(float height)
{
    return juce::FontOptions(height, juce::Font::bold);
}
}
