#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
const auto accent = juce::Colour::fromRGB(0xe8, 0x73, 0x0a);
const auto blue = juce::Colour::fromRGB(0x4a, 0x9e, 0xff);
const auto cream = juce::Colour::fromRGB(0xe8, 0xe4, 0xd8);
const auto green = juce::Colour::fromRGB(0xa0, 0xc8, 0x78);

juce::Colour panelTop() { return juce::Colour::fromRGB(0x25, 0x25, 0x28); }
juce::Colour panelBottom() { return juce::Colour::fromRGB(0x14, 0x14, 0x18); }
juce::Colour stripColour() { return juce::Colour::fromRGB(0x11, 0x11, 0x14); }

juce::Font monoFont(float height)
{
    return juce::FontOptions("Menlo", height, juce::Font::plain);
}

juce::Font titleFont(float height)
{
    return juce::FontOptions(height, juce::Font::bold);
}
}

class DoubleOctaverAudioProcessorEditor::RotaryLookAndFeel
    : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                            static_cast<float>(width), static_cast<float>(height))
                          .reduced(5.0f);
        const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centre = bounds.getCentre();
        const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        const auto colour = slider.findColour(juce::Slider::thumbColourId);

        if (slider.isMouseButtonDown())
        {
            g.setColour(colour.withAlpha(0.22f));
            g.fillEllipse(bounds.expanded(5.0f));
        }

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

    void drawLabel(juce::Graphics& g, juce::Label& label) override
    {
        g.setColour(label.findColour(juce::Label::textColourId));
        g.setFont(monoFont(10.0f));
        g.drawFittedText(label.getText(), label.getLocalBounds(), juce::Justification::centred, 1);
    }
};

class DoubleOctaverAudioProcessorEditor::PowerButtonLookAndFeel
    : public juce::LookAndFeel_V4
{
public:
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override
    {
        juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

        auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
        const auto active = button.getToggleState();
        const auto colour = active ? accent : juce::Colours::white.withAlpha(0.16f);

        g.setColour(colour.withAlpha(active ? 0.18f : 0.04f));
        g.fillRoundedRectangle(bounds, 3.0f);
        g.setColour(colour.withAlpha(active ? 0.55f : 0.22f));
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

        g.setColour(active ? accent : juce::Colours::white.withAlpha(0.25f));
        g.setFont(monoFont(9.0f));
        g.drawFittedText(active ? "ON" : "OFF", button.getLocalBounds(),
                         juce::Justification::centred, 1);
    }
};

class DoubleOctaverAudioProcessorEditor::OctaveSelector
    : public juce::Component,
      private juce::Timer
{
public:
    OctaveSelector(juce::AudioProcessorValueTreeState& state, const juce::String& parameterId,
                   juce::String selectorName, juce::Colour accentColour)
        : parameter(state.getParameter(parameterId)),
          name(std::move(selectorName)),
          colour(accentColour)
    {
        jassert(parameter != nullptr);

        for (auto i = 0; i < buttons.size(); ++i)
        {
            auto& button = buttons[static_cast<size_t>(i)];
            button.setButtonText(options[static_cast<size_t>(i)]);
            button.setClickingTogglesState(false);
            button.onClick = [this, i] { setParameterIndex(i); };
            addAndMakeVisible(button);
        }

        startTimerHz(60);
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(cream.withAlpha(0.42f));
        g.setFont(monoFont(11.0f));
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
        g.setFont(monoFont(10.0f));
        g.drawFittedText(options[static_cast<size_t>(selectedIndex)] + " OCT",
                         getLocalBounds().removeFromBottom(16),
                         juce::Justification::centred, 1);
    }

    void resized() override
    {
        auto selectorBounds = getSelectorBounds().reduced(4);
        const auto buttonHeight = selectorBounds.getHeight() / 4;

        for (auto i = 0; i < buttons.size(); ++i)
            buttons[static_cast<size_t>(i)].setBounds(selectorBounds.removeFromTop(buttonHeight).reduced(1));
    }

private:
    void timerCallback() override
    {
        const auto nextIndex = getParameterIndex();
        if (nextIndex != selectedIndex)
            selectedIndex = nextIndex;

        const auto target = static_cast<float>(selectedIndex);
        indicatorPosition += (target - indicatorPosition) * 0.24f;

        for (auto i = 0; i < buttons.size(); ++i)
        {
            auto& button = buttons[static_cast<size_t>(i)];
            const auto active = i == selectedIndex;
            button.setToggleState(active, juce::dontSendNotification);
            button.setColour(juce::TextButton::buttonColourId,
                             active ? colour.withAlpha(0.20f) : juce::Colour::fromRGB(0x24, 0x24, 0x2c));
            button.setColour(juce::TextButton::buttonOnColourId, colour.withAlpha(0.20f));
            button.setColour(juce::TextButton::textColourOffId, cream.withAlpha(0.48f));
            button.setColour(juce::TextButton::textColourOnId, colour);
        }

        repaint();
    }

    juce::Rectangle<int> getSelectorBounds() const
    {
        auto bounds = getLocalBounds().withTrimmedTop(19).withTrimmedBottom(15);
        return bounds.withSizeKeepingCentre(58, juce::jmax(56, bounds.getHeight()));
    }

    int getParameterIndex() const
    {
        if (parameter == nullptr)
            return 0;

        return juce::jlimit(0, 3, static_cast<int>(std::round(parameter->convertFrom0to1(parameter->getValue()))));
    }

    void setParameterIndex(int index)
    {
        if (parameter == nullptr)
            return;

        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(static_cast<float>(index)));
        parameter->endChangeGesture();
    }

    juce::RangedAudioParameter* parameter = nullptr;
    juce::String name;
    juce::Colour colour;
    std::array<juce::TextButton, 4> buttons;
    std::array<juce::String, 4> options { "-2", "-1", "+1", "+2" };
    int selectedIndex = 0;
    float indicatorPosition = 0.0f;
};

DoubleOctaverAudioProcessorEditor::DoubleOctaverAudioProcessorEditor (DoubleOctaverAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    rotaryLookAndFeel = new RotaryLookAndFeel();
    powerButtonLookAndFeel = new PowerButtonLookAndFeel();

    configureSlider(octaveGainSlider, " dB");
    configureSlider(octaveGain2Slider, " dB");
    configureSlider(dryWetSlider, "%");
    configureSlider(gainSlider, " dB");

    octaveGainSlider.setColour(juce::Slider::thumbColourId, accent);
    octaveGain2Slider.setColour(juce::Slider::thumbColourId, blue);
    dryWetSlider.setColour(juce::Slider::thumbColourId, cream);
    gainSlider.setColour(juce::Slider::thumbColourId, green);

    addAndMakeVisible(octaveGainSlider);
    addAndMakeVisible(octaveGain2Slider);
    addAndMakeVisible(dryWetSlider);
    addAndMakeVisible(gainSlider);

    octaveSelector = new OctaveSelector(audioProcessor.apvts, "OctaveShift", "Voice 1", accent);
    octaveSelector2 = new OctaveSelector(audioProcessor.apvts, "OctaveShift2", "Voice 2", blue);
    addAndMakeVisible(octaveSelector);
    addAndMakeVisible(octaveSelector2);

    powerButton.setLookAndFeel(powerButtonLookAndFeel);
    powerButton.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(powerButton);

    octaveGainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "OctaveGain", octaveGainSlider);
    octaveGain2Attachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "OctaveGain2", octaveGain2Slider);
    dryWetAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DryWet", dryWetSlider);
    gainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Gain", gainSlider);
    powerAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "Power", powerButton);

    setSize (330, 380);
    startTimerHz(60);
}

DoubleOctaverAudioProcessorEditor::~DoubleOctaverAudioProcessorEditor()
{
    stopTimer();

    powerButton.setLookAndFeel(nullptr);
    octaveGainSlider.setLookAndFeel(nullptr);
    octaveGain2Slider.setLookAndFeel(nullptr);
    dryWetSlider.setLookAndFeel(nullptr);
    gainSlider.setLookAndFeel(nullptr);

    delete octaveSelector;
    delete octaveSelector2;
    delete powerButtonLookAndFeel;
    delete rotaryLookAndFeel;
}

void DoubleOctaverAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(0x0f, 0x0f, 0x12));

    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    juce::ColourGradient panel(panelTop(), bounds.getX(), bounds.getY(),
                               panelBottom(), bounds.getRight(), bounds.getBottom(), false);
    g.setGradientFill(panel);
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(juce::Colours::white.withAlpha(0.09f));
    g.drawRoundedRectangle(bounds, 8.0f, 1.0f);

    auto header = getLocalBounds().removeFromTop(42).toFloat();
    juce::ColourGradient headerGradient(stripColour(), header.getX(), header.getCentreY(),
                                        juce::Colour::fromRGB(0x1c, 0x1c, 0x21),
                                        header.getCentreX(), header.getCentreY(), false);
    g.setGradientFill(headerGradient);
    g.fillRect(header);
    g.setColour(juce::Colours::white.withAlpha(0.06f));
    g.drawHorizontalLine(42, 0.0f, static_cast<float>(getWidth()));

    g.setColour(accent);
    g.setFont(titleFont(18.0f));
    g.drawFittedText("OCTAVER", 14, 10, 130, 20, juce::Justification::centredLeft, 1);
    g.setColour(cream.withAlpha(0.30f));
    g.setFont(monoFont(9.0f));
    g.drawFittedText("v1.1", 109, 15, 45, 14, juce::Justification::centredLeft, 1);

    const auto powerOn = audioProcessor.apvts.getRawParameterValue("Power")->load() > 0.5f;
    auto led = juce::Rectangle<float>(getWidth() - 83.0f, 18.0f, 6.0f, 6.0f);
    g.setColour(powerOn ? accent.withAlpha(0.25f) : juce::Colours::transparentBlack);
    g.fillEllipse(led.expanded(4.0f));
    g.setColour(powerOn ? accent : juce::Colour::fromRGB(0x33, 0x33, 0x33));
    g.fillEllipse(led);

    g.setColour(juce::Colours::white.withAlpha(0.18f));
    for (auto x : { 14.0f, 112.0f, 218.0f, static_cast<float>(getWidth() - 22) })
    {
        g.fillEllipse(x, 51.0f, 8.0f, 8.0f);
        g.setColour(juce::Colours::black.withAlpha(0.45f));
        g.drawLine(x + 2.0f, 55.0f, x + 6.0f, 55.0f, 1.0f);
        g.setColour(juce::Colours::white.withAlpha(0.18f));
    }

    g.setColour(cream.withAlpha(0.24f));
    g.setFont(monoFont(8.5f));
    g.drawFittedText("OCTAVE VOICES", 18, 70, 210, 16, juce::Justification::centredLeft, 1);
    g.drawFittedText("OUTPUT", 236, 70, 74, 16, juce::Justification::centredLeft, 1);

    g.setColour(cream.withAlpha(0.50f));
    g.setFont(monoFont(9.0f));
    g.drawFittedText("OCT 1", 24, 91, 76, 12, juce::Justification::centred, 1);
    g.drawFittedText("OCT 2", 119, 91, 76, 12, juce::Justification::centred, 1);
    g.drawFittedText("MIX", 236, 91, 76, 12, juce::Justification::centred, 1);
    g.drawFittedText("MASTER", 236, 216, 76, 12, juce::Justification::centred, 1);

    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.drawHorizontalLine(90, 18.0f, 226.0f);
    g.drawHorizontalLine(90, 236.0f, static_cast<float>(getWidth() - 20));

    juce::ColourGradient divider(juce::Colours::transparentBlack, 219.0f, 65.0f,
                                 juce::Colours::white.withAlpha(0.08f), 219.0f, 150.0f,
                                 false);
    g.setGradientFill(divider);
    g.drawVerticalLine(220, 65.0f, static_cast<float>(getHeight() - 42));

    auto footer = getLocalBounds().removeFromBottom(30).toFloat();
    g.setColour(stripColour());
    g.fillRect(footer);
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.drawHorizontalLine(getHeight() - 30, 0.0f, static_cast<float>(getWidth()));

    g.setColour(cream.withAlpha(0.18f));
    g.setFont(monoFont(8.0f));
    g.drawFittedText("PITCH SHIFT ENGINE", 14, getHeight() - 22, 126, 14,
                     juce::Justification::centredLeft, 1);

    drawStatusDot(g, { 180.0f, static_cast<float>(getHeight() - 15) }, cream, true, "DRY");
    drawStatusDot(g, { 217.0f, static_cast<float>(getHeight() - 15) }, accent, powerOn, "V1");
    drawStatusDot(g, { 250.0f, static_cast<float>(getHeight() - 15) }, blue, powerOn, "V2");
    drawStatusDot(g, { 283.0f, static_cast<float>(getHeight() - 15) }, green, powerOn, "OUT");
}

void DoubleOctaverAudioProcessorEditor::resized()
{
    powerButton.setBounds(getWidth() - 61, 12, 38, 18);

    octaveGainSlider.setBounds(24, 96, 76, 94);
    octaveSelector->setBounds(15, 194, 94, 150);

    octaveGain2Slider.setBounds(119, 96, 76, 94);
    octaveSelector2->setBounds(110, 194, 94, 150);

    dryWetSlider.setBounds(236, 106, 76, 92);
    gainSlider.setBounds(236, 231, 76, 92);
}

void DoubleOctaverAudioProcessorEditor::timerCallback()
{
    const auto powerOn = audioProcessor.apvts.getRawParameterValue("Power")->load() > 0.5f;
    const auto targetAlpha = powerOn ? 1.0f : 0.35f;
    controlsAlpha += (targetAlpha - controlsAlpha) * 0.18f;

    octaveGainSlider.setAlpha(controlsAlpha);
    octaveGain2Slider.setAlpha(controlsAlpha);
    dryWetSlider.setAlpha(controlsAlpha);
    gainSlider.setAlpha(controlsAlpha);
    octaveSelector->setAlpha(controlsAlpha);
    octaveSelector2->setAlpha(controlsAlpha);

    repaint();
}

void DoubleOctaverAudioProcessorEditor::configureSlider(juce::Slider& slider,
                                                        const juce::String& suffix)
{
    slider.setLookAndFeel(rotaryLookAndFeel);
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 16);
    slider.setRotaryParameters(juce::degreesToRadians(215.0f),
                               juce::degreesToRadians(505.0f), true);
    slider.setTextValueSuffix(suffix);
    slider.setColour(juce::Slider::textBoxTextColourId, cream.withAlpha(0.8f));
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
}

void DoubleOctaverAudioProcessorEditor::drawStatusDot(juce::Graphics& g,
                                                      juce::Point<float> centre,
                                                      juce::Colour colour,
                                                      bool active,
                                                      const juce::String& label)
{
    g.setColour(active ? colour.withAlpha(0.25f) : juce::Colours::transparentBlack);
    g.fillEllipse(centre.x - 4.0f, centre.y - 4.0f, 8.0f, 8.0f);
    g.setColour(active ? colour : juce::Colour::fromRGB(0x33, 0x33, 0x33));
    g.fillEllipse(centre.x - 2.0f, centre.y - 2.0f, 4.0f, 4.0f);

    g.setColour(active ? colour.withAlpha(0.60f) : juce::Colours::white.withAlpha(0.12f));
    g.setFont(monoFont(7.0f));
    g.drawFittedText(label, static_cast<int>(centre.x + 5.0f),
                     static_cast<int>(centre.y - 5.0f), 24, 10,
                     juce::Justification::centredLeft, 1);
}
