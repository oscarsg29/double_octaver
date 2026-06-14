#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParameterIds.h"
#include "GUI/Layout.h"
#include "GUI/LookAndFeels.h"
#include "GUI/SliderConfig.h"
#include "GUI/Theme.h"
#include "GUI/VoiceControls.h"

namespace gui = double_octaver::gui;
namespace parameters = double_octaver::parameters;

DoubleOctaverAudioProcessorEditor::DoubleOctaverAudioProcessorEditor (DoubleOctaverAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    rotaryLookAndFeel = std::make_unique<gui::RotaryLookAndFeel>();
    powerButtonLookAndFeel = std::make_unique<gui::PowerButtonLookAndFeel>();
    bypassButtonLookAndFeel = std::make_unique<gui::BypassButtonLookAndFeel>();

    configureSlider(dryWetSlider, "%", gui::theme::cream);
    configureSlider(gainSlider, " dB", gui::theme::green);

    addAndMakeVisible(dryWetSlider);
    addAndMakeVisible(gainSlider);

    voice1Controls = std::make_unique<gui::VoiceControls>(audioProcessor.apvts,
                                                          parameters::octaveGain1,
                                                          parameters::octaveShift1,
                                                          parameters::octaveBypass1,
                                                          "Voice 1",
                                                          gui::theme::accent,
                                                          *rotaryLookAndFeel,
                                                          *bypassButtonLookAndFeel);
    voice2Controls = std::make_unique<gui::VoiceControls>(audioProcessor.apvts,
                                                          parameters::octaveGain2,
                                                          parameters::octaveShift2,
                                                          parameters::octaveBypass2,
                                                          "Voice 2",
                                                          gui::theme::blue,
                                                          *rotaryLookAndFeel,
                                                          *bypassButtonLookAndFeel);
    addAndMakeVisible(*voice1Controls);
    addAndMakeVisible(*voice2Controls);

    powerButton.setLookAndFeel(powerButtonLookAndFeel.get());
    powerButton.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(powerButton);

    dryWetAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts,
                                                          parameters::dryWet,
                                                          dryWetSlider);
    gainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts,
                                                        parameters::gain,
                                                        gainSlider);
    powerAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts,
                                                        parameters::power,
                                                        powerButton);

    setSize (gui::layout::editorWidth, gui::layout::editorHeight);
    startTimerHz(60);
}

DoubleOctaverAudioProcessorEditor::~DoubleOctaverAudioProcessorEditor()
{
    stopTimer();

    voice1Controls.reset();
    voice2Controls.reset();
    powerButton.setLookAndFeel(nullptr);
    dryWetSlider.setLookAndFeel(nullptr);
    gainSlider.setLookAndFeel(nullptr);

    bypassButtonLookAndFeel.reset();
    powerButtonLookAndFeel.reset();
    rotaryLookAndFeel.reset();
}

void DoubleOctaverAudioProcessorEditor::paint (juce::Graphics& g)
{
    const auto powerOn = audioProcessor.apvts.getRawParameterValue(parameters::power)->load() > 0.5f;
    const auto voice1On = powerOn && voice1Controls->isActive();
    const auto voice2On = powerOn && voice2Controls->isActive();

    drawPanel(g);
    drawHeader(g, powerOn);
    drawSectionLabels(g);
    drawFooter(g, powerOn, voice1On, voice2On);
}

void DoubleOctaverAudioProcessorEditor::drawPanel(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(0x0f, 0x0f, 0x12));

    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    juce::ColourGradient panel(gui::theme::panelTop(), bounds.getX(), bounds.getY(),
                               gui::theme::panelBottom(), bounds.getRight(), bounds.getBottom(), false);
    g.setGradientFill(panel);
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(juce::Colours::white.withAlpha(0.09f));
    g.drawRoundedRectangle(bounds, 8.0f, 1.0f);
}

void DoubleOctaverAudioProcessorEditor::drawHeader(juce::Graphics& g, bool powerOn)
{
    auto header = getLocalBounds().removeFromTop(gui::layout::headerHeight).toFloat();
    juce::ColourGradient headerGradient(gui::theme::stripColour(), header.getX(), header.getCentreY(),
                                        juce::Colour::fromRGB(0x1c, 0x1c, 0x21),
                                        header.getCentreX(), header.getCentreY(), false);
    g.setGradientFill(headerGradient);
    g.fillRect(header);
    g.setColour(juce::Colours::white.withAlpha(0.06f));
    g.drawHorizontalLine(gui::layout::headerHeight, 0.0f, static_cast<float>(getWidth()));

    g.setColour(gui::theme::accent);
    g.setFont(gui::theme::titleFont(18.0f));
    g.drawFittedText("DOUBLE OCTAVER", gui::layout::titleX, gui::layout::titleY,
                     gui::layout::titleWidth, gui::layout::titleHeight,
                     juce::Justification::centredLeft, 1);

    auto led = juce::Rectangle<float>(getWidth() - gui::layout::powerLedRightMargin,
                                      gui::layout::powerLedY,
                                      gui::layout::powerLedSize,
                                      gui::layout::powerLedSize);
    g.setColour(powerOn ? gui::theme::accent.withAlpha(0.25f) : juce::Colours::transparentBlack);
    g.fillEllipse(led.expanded(4.0f));
    g.setColour(powerOn ? gui::theme::accent : juce::Colour::fromRGB(0x33, 0x33, 0x33));
    g.fillEllipse(led);

    g.setColour(juce::Colours::white.withAlpha(0.18f));
    for (auto x : { gui::layout::screw1X,
                    gui::layout::screw2X,
                    gui::layout::screw3X,
                    static_cast<float>(getWidth() - gui::layout::screwRightMargin) })
    {
        g.fillEllipse(x, gui::layout::screwY, gui::layout::screwSize, gui::layout::screwSize);
        g.setColour(juce::Colours::black.withAlpha(0.45f));
        g.drawLine(x + gui::layout::screwSlotInset,
                   gui::layout::screwSlotY,
                   x + gui::layout::screwSize - gui::layout::screwSlotInset,
                   gui::layout::screwSlotY,
                   1.0f);
        g.setColour(juce::Colours::white.withAlpha(0.18f));
    }
}

void DoubleOctaverAudioProcessorEditor::drawSectionLabels(juce::Graphics& g)
{
    g.setColour(gui::theme::cream.withAlpha(0.24f));
    g.setFont(gui::theme::monoFont(8.5f));
    g.drawFittedText("OCTAVE VOICES", gui::layout::voicesSectionLabelX,
                     gui::layout::sectionLabelY, gui::layout::voicesSectionLabelWidth,
                     gui::layout::sectionLabelHeight, juce::Justification::centredLeft, 1);
    g.drawFittedText("OUTPUT", gui::layout::outputX, gui::layout::sectionLabelY,
                     gui::layout::outputSectionLabelWidth, gui::layout::sectionLabelHeight,
                     juce::Justification::centredLeft, 1);

    g.setColour(gui::theme::cream.withAlpha(0.50f));
    g.setFont(gui::theme::monoFont(9.0f));
    g.drawFittedText("GAIN", gui::layout::voice1X + gui::layout::voiceKnobX, gui::layout::controlLabelY,
                     gui::layout::knobWidth, gui::layout::labelHeight, juce::Justification::centred, 1);
    g.drawFittedText("GAIN", gui::layout::voice2X + gui::layout::voiceKnobX, gui::layout::controlLabelY,
                     gui::layout::knobWidth, gui::layout::labelHeight, juce::Justification::centred, 1);
    g.drawFittedText("MIX", gui::layout::outputX, gui::layout::controlLabelY,
                     gui::layout::knobWidth, gui::layout::labelHeight, juce::Justification::centred, 1);
    g.drawFittedText("MASTER", gui::layout::outputX, gui::layout::masterLabelY,
                     gui::layout::knobWidth, gui::layout::labelHeight, juce::Justification::centred, 1);

    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.drawHorizontalLine(gui::layout::sectionRuleY,
                         gui::layout::voicesRuleStartX,
                         gui::layout::voicesRuleEndX);
    g.drawHorizontalLine(gui::layout::sectionRuleY,
                         gui::layout::outputRuleStartX,
                         static_cast<float>(getWidth() - gui::layout::outputRuleRightMargin));

    juce::ColourGradient divider(juce::Colours::transparentBlack,
                                 gui::layout::dividerGradientX,
                                 gui::layout::dividerGradientTopY,
                                 juce::Colours::white.withAlpha(0.08f),
                                 gui::layout::dividerGradientX,
                                 gui::layout::dividerGradientBottomY,
                                 false);
    g.setGradientFill(divider);
    g.drawVerticalLine(gui::layout::dividerX,
                       gui::layout::dividerTopY,
                       static_cast<float>(getHeight() - gui::layout::dividerBottomMargin));
}

void DoubleOctaverAudioProcessorEditor::drawFooter(juce::Graphics& g, bool powerOn,
                                                   bool voice1On, bool voice2On)
{
    auto footer = getLocalBounds().removeFromBottom(gui::layout::footerHeight).toFloat();
    g.setColour(gui::theme::stripColour());
    g.fillRect(footer);
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.drawHorizontalLine(getHeight() - gui::layout::footerHeight, 0.0f, static_cast<float>(getWidth()));

    g.setColour(gui::theme::cream.withAlpha(0.18f));
    g.setFont(gui::theme::monoFont(8.0f));
    g.drawFittedText("PITCH SHIFT ENGINE", gui::layout::footerTextX,
                     getHeight() - gui::layout::footerTextBottomMargin,
                     gui::layout::footerTextWidth, gui::layout::footerTextHeight,
                     juce::Justification::centredLeft, 1);

    const auto statusY = static_cast<float>(getHeight() - gui::layout::statusDotBottomMargin);
    drawStatusDot(g, { gui::layout::dryStatusDotX, statusY }, gui::theme::cream, true, "DRY");
    drawStatusDot(g, { gui::layout::voice1StatusDotX, statusY }, gui::theme::accent, voice1On, "V1");
    drawStatusDot(g, { gui::layout::voice2StatusDotX, statusY }, gui::theme::blue, voice2On, "V2");
    drawStatusDot(g, { gui::layout::outputStatusDotX, statusY }, gui::theme::green, powerOn, "OUT");
}

void DoubleOctaverAudioProcessorEditor::resized()
{
    powerButton.setBounds(getWidth() - gui::layout::powerButtonRightMargin, gui::layout::powerButtonY,
                          gui::layout::powerButtonWidth, gui::layout::powerButtonHeight);

    voice1Controls->setBounds(gui::layout::voice1X, gui::layout::voiceY,
                              gui::layout::voiceWidth, gui::layout::voiceHeight);
    voice2Controls->setBounds(gui::layout::voice2X, gui::layout::voiceY,
                              gui::layout::voiceWidth, gui::layout::voiceHeight);

    dryWetSlider.setBounds(gui::layout::outputX, gui::layout::mixY,
                           gui::layout::knobWidth, gui::layout::outputKnobHeight);
    gainSlider.setBounds(gui::layout::outputX, gui::layout::masterY,
                         gui::layout::knobWidth, gui::layout::outputKnobHeight);
}

void DoubleOctaverAudioProcessorEditor::timerCallback()
{
    const auto powerOn = audioProcessor.apvts.getRawParameterValue(parameters::power)->load() > 0.5f;
    const auto targetAlpha = powerOn ? 1.0f : 0.35f;
    controlsAlpha += (targetAlpha - controlsAlpha) * 0.18f;

    voice1Controls->setPowerAlpha(controlsAlpha);
    voice2Controls->setPowerAlpha(controlsAlpha);
    dryWetSlider.setAlpha(controlsAlpha);
    gainSlider.setAlpha(controlsAlpha);

    repaint();
}

void DoubleOctaverAudioProcessorEditor::configureSlider(juce::Slider& slider,
                                                        const juce::String& suffix,
                                                        juce::Colour thumbColour)
{
    gui::configureRotarySlider(slider, *rotaryLookAndFeel, suffix, thumbColour);
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
    g.setFont(gui::theme::monoFont(7.0f));
    g.drawFittedText(label, static_cast<int>(centre.x + 5.0f),
                     static_cast<int>(centre.y - 5.0f), 24, 10,
                     juce::Justification::centredLeft, 1);
}
