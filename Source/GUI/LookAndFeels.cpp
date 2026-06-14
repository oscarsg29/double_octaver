#include "LookAndFeels.h"
#include "Theme.h"

namespace double_octaver::gui
{
void RotaryLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float rotaryStartAngle,
                                         float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                        static_cast<float>(width), static_cast<float>(height))
                      .reduced(5.0f);
    const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const auto colour = slider.findColour(juce::Slider::thumbColourId);

    juce::ColourGradient outer(juce::Colour::fromRGB(0x3c, 0x3c, 0x46),
                               centre.x - radius * 0.3f, centre.y - radius * 0.4f,
                               juce::Colour::fromRGB(0x18, 0x18, 0x1e),
                               centre.x + radius * 0.4f, centre.y + radius * 0.5f,
                               true);

    g.setGradientFill(outer);
    g.fillEllipse(bounds);
    g.setColour(juce::Colours::white.withAlpha(0.07f));
    g.drawEllipse(bounds, 1.0f);

    juce::Path track;
    track.addCentredArc(centre.x, centre.y, radius - 6.0f, radius - 6.0f,
                        0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.strokePath(track, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                            juce::PathStrokeType::rounded));

    juce::Path valueArc;
    valueArc.addCentredArc(centre.x, centre.y, radius - 6.0f, radius - 6.0f,
                           0.0f, rotaryStartAngle, angle, true);
    g.setColour(colour.withAlpha(0.9f));
    g.strokePath(valueArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

    auto inner = bounds.reduced(10.0f);
    juce::ColourGradient innerGradient(juce::Colour::fromRGB(0x38, 0x38, 0x40),
                                       inner.getX(), inner.getY(),
                                       juce::Colour::fromRGB(0x18, 0x18, 0x1e),
                                       inner.getRight(), inner.getBottom(), true);
    g.setGradientFill(innerGradient);
    g.fillEllipse(inner);
    g.setColour(juce::Colours::black.withAlpha(0.55f));
    g.drawEllipse(inner, 1.0f);

    juce::Path pointer;
    const auto pointerLength = radius - 14.0f;
    pointer.addRoundedRectangle(-1.25f, -pointerLength, 2.5f, pointerLength, 1.25f);
    pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
    g.setColour(colour);
    g.fillPath(pointer);

    g.setColour(colour.withAlpha(0.55f));
    g.fillEllipse(centre.x - 2.5f, centre.y - 2.5f, 5.0f, 5.0f);
}

void RotaryLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.setColour(label.findColour(juce::Label::textColourId));
    g.setFont(theme::monoFont(10.0f));
    g.drawFittedText(label.getText(), label.getLocalBounds(), juce::Justification::centred, 1);
}

void PowerButtonLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                              bool shouldDrawButtonAsHighlighted,
                                              bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    const auto active = button.getToggleState();
    const auto colour = active ? theme::accent : juce::Colours::white.withAlpha(0.16f);

    g.setColour(colour.withAlpha(active ? 0.18f : 0.04f));
    g.fillRoundedRectangle(bounds, 3.0f);
    g.setColour(colour.withAlpha(active ? 0.55f : 0.22f));
    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

    g.setColour(active ? theme::accent : juce::Colours::white.withAlpha(0.25f));
    g.setFont(theme::monoFont(9.0f));
    g.drawFittedText(active ? "ON" : "OFF", button.getLocalBounds(),
                     juce::Justification::centred, 1);
}

void BypassButtonLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                               bool shouldDrawButtonAsHighlighted,
                                               bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    const auto active = button.getToggleState();
    const auto colour = button.findColour(juce::ToggleButton::tickColourId);

    g.setColour(active ? colour.withAlpha(0.24f) : juce::Colour::fromRGB(0x18, 0x18, 0x1f));
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(active ? colour.withAlpha(0.70f) : juce::Colours::white.withAlpha(0.10f));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    g.setColour(active ? colour : theme::cream.withAlpha(0.42f));
    g.setFont(theme::monoFont(8.5f));
    g.drawFittedText("BYPASS", button.getLocalBounds(), juce::Justification::centred, 1);
}
}
