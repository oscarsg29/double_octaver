#include "OctaveSelector.h"
#include "Theme.h"

namespace double_octaver::gui
{
OctaveSelector::OctaveSelector(juce::AudioProcessorValueTreeState& state,
                               const juce::String& parameterId,
                               juce::String selectorName,
                               juce::Colour accentColour)
    : name(std::move(selectorName)),
      colour(accentColour)
{
    auto* parameter = state.getParameter(parameterId);
    jassert(parameter != nullptr);

    if (parameter != nullptr)
    {
        parameterAttachment = std::make_unique<juce::ParameterAttachment>(
            *parameter,
            [this](float value) { updateSelectedIndex(value); });
    }

    for (auto i = 0; i < buttons.size(); ++i)
    {
        auto& button = buttons[static_cast<size_t>(i)];
        button.setButtonText(options[static_cast<size_t>(i)]);
        button.setClickingTogglesState(false);
        button.onClick = [this, i] { setParameterIndex(i); };
        addAndMakeVisible(button);
    }

    if (parameterAttachment != nullptr)
        parameterAttachment->sendInitialUpdate();

    startTimerHz(30);
}

void OctaveSelector::paint(juce::Graphics& g)
{
    g.setColour(theme::cream.withAlpha(0.42f));
    g.setFont(theme::monoFont(11.0f));
    g.drawFittedText(name.toUpperCase(), getLocalBounds().removeFromTop(18),
                     juce::Justification::centred, 1);

    auto selectorBounds = getSelectorBounds().toFloat();
    juce::ColourGradient housing(juce::Colour::fromRGB(0x11, 0x11, 0x16),
                                 selectorBounds.getCentreX(), selectorBounds.getY(),
                                 juce::Colour::fromRGB(0x1c, 0x1c, 0x22),
                                 selectorBounds.getCentreX(), selectorBounds.getBottom(),
                                 false);
    g.setGradientFill(housing);
    g.fillRoundedRectangle(selectorBounds, 6.0f);
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawRoundedRectangle(selectorBounds, 6.0f, 1.0f);

    const auto buttonHeight = selectorBounds.getHeight() / 4.0f;
    const auto indicatorY = selectorBounds.getY() + 4.0f + indicatorPosition * buttonHeight
                            + buttonHeight * 0.5f - 4.5f;
    auto indicator = juce::Rectangle<float>(selectorBounds.getX() - 7.0f, indicatorY, 3.0f, 9.0f);
    g.setColour(colour.withAlpha(0.35f));
    g.fillRoundedRectangle(indicator.expanded(2.0f), 3.0f);
    g.setColour(colour);
    g.fillRoundedRectangle(indicator, 2.0f);

    g.setColour(colour.withAlpha(0.78f));
    g.setFont(theme::monoFont(10.0f));
}

void OctaveSelector::resized()
{
    auto selectorBounds = getSelectorBounds().reduced(4);
    const auto buttonHeight = selectorBounds.getHeight() / 4;

    for (auto i = 0; i < buttons.size(); ++i)
        buttons[static_cast<size_t>(i)].setBounds(selectorBounds.removeFromTop(buttonHeight).reduced(1));
}

void OctaveSelector::timerCallback()
{
    const auto target = static_cast<float>(selectedIndex);
    indicatorPosition += (target - indicatorPosition) * 0.24f;

    repaint();
}

juce::Rectangle<int> OctaveSelector::getSelectorBounds() const
{
    auto bounds = getLocalBounds().withTrimmedTop(19).withTrimmedBottom(15);
    return bounds.withSizeKeepingCentre(58, juce::jmax(56, bounds.getHeight()));
}

void OctaveSelector::setParameterIndex(int index)
{
    if (parameterAttachment == nullptr)
        return;

    parameterAttachment->setValueAsCompleteGesture(static_cast<float>(index));
}

void OctaveSelector::updateSelectedIndex(float index)
{
    selectedIndex = juce::jlimit(0, 3, static_cast<int>(std::round(index)));

    for (auto i = 0; i < buttons.size(); ++i)
    {
        auto& button = buttons[static_cast<size_t>(i)];
        const auto active = i == selectedIndex;
        button.setToggleState(active, juce::dontSendNotification);
        button.setColour(juce::TextButton::buttonColourId,
                         active ? colour.withAlpha(0.20f) : juce::Colour::fromRGB(0x24, 0x24, 0x2c));
        button.setColour(juce::TextButton::buttonOnColourId, colour.withAlpha(0.20f));
        button.setColour(juce::TextButton::textColourOffId, theme::cream.withAlpha(0.48f));
        button.setColour(juce::TextButton::textColourOnId, colour);
    }

    repaint();
}
}
