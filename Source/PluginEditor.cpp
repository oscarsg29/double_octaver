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
    auto& parameterState = audioProcessor.getValueTreeState();

    rotaryLookAndFeel = std::make_unique<gui::RotaryLookAndFeel>();
    powerButtonLookAndFeel = std::make_unique<gui::PowerButtonLookAndFeel>();
    bypassButtonLookAndFeel = std::make_unique<gui::BypassButtonLookAndFeel>();

    configureSlider(dryWetSlider, "%", gui::theme::cream);
    configureSlider(gainSlider, " dB", gui::theme::green);

    addAndMakeVisible(dryWetSlider);
    addAndMakeVisible(gainSlider);

    voice1Controls = std::make_unique<gui::VoiceControls>(parameterState,
                                                          parameters::octaveGain1,
                                                          parameters::octaveShift1,
                                                          parameters::octaveBypass1,
                                                          "Voice 1",
                                                          gui::theme::accent,
                                                          *rotaryLookAndFeel,
                                                          *bypassButtonLookAndFeel);
    voice2Controls = std::make_unique<gui::VoiceControls>(parameterState,
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

    dryWetAttachment = std::make_unique<SliderAttachment>(parameterState,
                                                          parameters::dryWet,
                                                          dryWetSlider);
    gainAttachment = std::make_unique<SliderAttachment>(parameterState,
                                                        parameters::gain,
                                                        gainSlider);
    powerAttachment = std::make_unique<ButtonAttachment>(parameterState,
                                                        parameters::power,
                                                        powerButton);

    setSize (gui::layout::editor::width, gui::layout::editor::height);
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
    const auto powerOn = audioProcessor.getValueTreeState().getRawParameterValue(parameters::power)->load() > 0.5f;
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
    auto header = getLocalBounds().removeFromTop(gui::layout::header::height).toFloat();
    juce::ColourGradient headerGradient(gui::theme::stripColour(), header.getX(), header.getCentreY(),
                                        juce::Colour::fromRGB(0x1c, 0x1c, 0x21),
                                        header.getCentreX(), header.getCentreY(), false);
    g.setGradientFill(headerGradient);
    g.fillRect(header);
    g.setColour(juce::Colours::white.withAlpha(0.06f));
    g.drawHorizontalLine(gui::layout::header::height, 0.0f, static_cast<float>(getWidth()));

    g.setColour(gui::theme::accent);
    g.setFont(gui::theme::titleFont(18.0f));
    g.drawFittedText("DOUBLE OCTAVER", gui::layout::header::titleX, gui::layout::header::titleY,
                     gui::layout::header::titleWidth, gui::layout::header::titleHeight,
                     juce::Justification::centredLeft, 1);

    auto led = juce::Rectangle<float>(getWidth() - gui::layout::header::powerLedRightMargin,
                                      gui::layout::header::powerLedY,
                                      gui::layout::header::powerLedSize,
                                      gui::layout::header::powerLedSize);
    g.setColour(powerOn ? gui::theme::accent.withAlpha(0.25f) : juce::Colours::transparentBlack);
    g.fillEllipse(led.expanded(4.0f));
    g.setColour(powerOn ? gui::theme::accent : juce::Colour::fromRGB(0x33, 0x33, 0x33));
    g.fillEllipse(led);

    g.setColour(juce::Colours::white.withAlpha(0.18f));
    for (auto x : { gui::layout::header::screw1X,
                    gui::layout::header::screw2X,
                    gui::layout::header::screw3X,
                    static_cast<float>(getWidth() - gui::layout::header::screwRightMargin) })
    {
        g.fillEllipse(x, gui::layout::header::screwY,
                      gui::layout::header::screwSize, gui::layout::header::screwSize);
        g.setColour(juce::Colours::black.withAlpha(0.45f));
        g.drawLine(x + gui::layout::header::screwSlotInset,
                   gui::layout::header::screwSlotY,
                   x + gui::layout::header::screwSize - gui::layout::header::screwSlotInset,
                   gui::layout::header::screwSlotY,
                   1.0f);
        g.setColour(juce::Colours::white.withAlpha(0.18f));
    }
}

void DoubleOctaverAudioProcessorEditor::drawSectionLabels(juce::Graphics& g)
{
    g.setColour(gui::theme::cream.withAlpha(0.24f));
    g.setFont(gui::theme::monoFont(8.5f));
    g.drawFittedText("OCTAVE VOICES", gui::layout::voices::sectionLabelX,
                     gui::layout::sections::labelY, gui::layout::voices::sectionLabelWidth,
                     gui::layout::sections::labelHeight, juce::Justification::centredLeft, 1);
    g.drawFittedText("OUTPUT", gui::layout::output::x, gui::layout::sections::labelY,
                     gui::layout::output::sectionLabelWidth, gui::layout::sections::labelHeight,
                     juce::Justification::centredLeft, 1);

    g.setColour(gui::theme::cream.withAlpha(0.50f));
    g.setFont(gui::theme::monoFont(9.0f));
    g.drawFittedText("GAIN", gui::layout::voices::firstX + gui::layout::voiceControl::knobX,
                     gui::layout::controls::labelY,
                     gui::layout::controls::knobWidth, gui::layout::controls::labelHeight,
                     juce::Justification::centred, 1);
    g.drawFittedText("GAIN", gui::layout::voices::secondX + gui::layout::voiceControl::knobX,
                     gui::layout::controls::labelY,
                     gui::layout::controls::knobWidth, gui::layout::controls::labelHeight,
                     juce::Justification::centred, 1);
    g.drawFittedText("MIX", gui::layout::output::x, gui::layout::controls::labelY,
                     gui::layout::controls::knobWidth, gui::layout::controls::labelHeight,
                     juce::Justification::centred, 1);
    g.drawFittedText("MASTER", gui::layout::output::x, gui::layout::controls::masterLabelY,
                     gui::layout::controls::knobWidth, gui::layout::controls::labelHeight,
                     juce::Justification::centred, 1);

    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.drawHorizontalLine(gui::layout::sections::ruleY,
                         gui::layout::voices::ruleStartX,
                         gui::layout::voices::ruleEndX);
    g.drawHorizontalLine(gui::layout::sections::ruleY,
                         gui::layout::output::ruleStartX,
                         static_cast<float>(getWidth() - gui::layout::output::ruleRightMargin));

    juce::ColourGradient divider(juce::Colours::transparentBlack,
                                 gui::layout::divider::gradientX,
                                 gui::layout::divider::gradientTopY,
                                 juce::Colours::white.withAlpha(0.08f),
                                 gui::layout::divider::gradientX,
                                 gui::layout::divider::gradientBottomY,
                                 false);
    g.setGradientFill(divider);
    g.drawVerticalLine(gui::layout::divider::x,
                       gui::layout::divider::topY,
                       static_cast<float>(getHeight() - gui::layout::divider::bottomMargin));
}

void DoubleOctaverAudioProcessorEditor::drawFooter(juce::Graphics& g, bool powerOn,
                                                   bool voice1On, bool voice2On)
{
    auto footer = getLocalBounds().removeFromBottom(gui::layout::footer::height).toFloat();
    g.setColour(gui::theme::stripColour());
    g.fillRect(footer);
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.drawHorizontalLine(getHeight() - gui::layout::footer::height, 0.0f, static_cast<float>(getWidth()));

    g.setColour(gui::theme::cream.withAlpha(0.18f));
    g.setFont(gui::theme::monoFont(8.0f));
    g.drawFittedText("PITCH SHIFT ENGINE", gui::layout::footer::textX,
                     getHeight() - gui::layout::footer::textBottomMargin,
                     gui::layout::footer::textWidth, gui::layout::footer::textHeight,
                     juce::Justification::centredLeft, 1);

    const auto statusY = static_cast<float>(getHeight() - gui::layout::status::dotBottomMargin);
    drawStatusDot(g, { gui::layout::status::dryX, statusY }, gui::theme::cream, true, "DRY");
    drawStatusDot(g, { gui::layout::status::voice1X, statusY }, gui::theme::accent, voice1On, "V1");
    drawStatusDot(g, { gui::layout::status::voice2X, statusY }, gui::theme::blue, voice2On, "V2");
    drawStatusDot(g, { gui::layout::status::outputX, statusY }, gui::theme::green, powerOn, "OUT");
}

void DoubleOctaverAudioProcessorEditor::resized()
{
    powerButton.setBounds(getWidth() - gui::layout::powerButton::rightMargin,
                          gui::layout::powerButton::y,
                          gui::layout::powerButton::width,
                          gui::layout::powerButton::height);

    voice1Controls->setBounds(gui::layout::voices::firstX, gui::layout::voices::y,
                              gui::layout::voices::width, gui::layout::voices::height);
    voice2Controls->setBounds(gui::layout::voices::secondX, gui::layout::voices::y,
                              gui::layout::voices::width, gui::layout::voices::height);

    dryWetSlider.setBounds(gui::layout::output::x, gui::layout::output::mixY,
                           gui::layout::controls::knobWidth, gui::layout::output::knobHeight);
    gainSlider.setBounds(gui::layout::output::x, gui::layout::output::masterY,
                         gui::layout::controls::knobWidth, gui::layout::output::knobHeight);
}

void DoubleOctaverAudioProcessorEditor::timerCallback()
{
    const auto powerOn = audioProcessor.getValueTreeState().getRawParameterValue(parameters::power)->load() > 0.5f;
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
