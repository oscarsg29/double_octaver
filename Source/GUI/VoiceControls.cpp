#include "VoiceControls.h"
#include "Layout.h"
#include "SliderConfig.h"

namespace double_octaver::gui
{
VoiceControls::VoiceControls(juce::AudioProcessorValueTreeState& state,
                             const juce::String& gainParameterId,
                             const juce::String& shiftParameterId,
                             const juce::String& bypassParameterId,
                             juce::String selectorName,
                             juce::Colour voiceColour,
                             RotaryLookAndFeel& rotaryLookAndFeel,
                             BypassButtonLookAndFeel& bypassButtonLookAndFeel)
    : bypassParameter(state.getRawParameterValue(bypassParameterId)),
      selector(std::make_unique<OctaveSelector>(state, shiftParameterId, std::move(selectorName), voiceColour))
{
    jassert(bypassParameter != nullptr);

    configureGainSlider(rotaryLookAndFeel, voiceColour);
    addAndMakeVisible(gainSlider);

    addAndMakeVisible(*selector);

    bypassButton.setLookAndFeel(&bypassButtonLookAndFeel);
    bypassButton.setColour(juce::ToggleButton::tickColourId, voiceColour);
    bypassButton.setClickingTogglesState(true);
    bypassButton.onStateChange = [this] { updateControlAlpha(); };
    addAndMakeVisible(bypassButton);

    gainAttachment = std::make_unique<SliderAttachment>(state, gainParameterId, gainSlider);
    bypassAttachment = std::make_unique<ButtonAttachment>(state, bypassParameterId, bypassButton);
}

VoiceControls::~VoiceControls()
{
    bypassButton.setLookAndFeel(nullptr);
    gainSlider.setLookAndFeel(nullptr);
}

void VoiceControls::resized()
{
    gainSlider.setBounds(layout::voiceControl::knobX, layout::voiceControl::knobY,
                         layout::controls::knobWidth, layout::voiceControl::gainKnobHeight);
    selector->setBounds(0, layout::voiceControl::selectorY,
                        layout::voices::width, layout::voiceControl::selectorHeight);
    bypassButton.setBounds(layout::voiceControl::bypassX, layout::voiceControl::bypassY,
                           layout::voiceControl::bypassWidth, layout::voiceControl::bypassHeight);
}

void VoiceControls::setPowerAlpha(float alpha)
{
    powerAlpha = alpha;
    updateControlAlpha();
}

bool VoiceControls::isActive() const
{
    return bypassParameter != nullptr && bypassParameter->load() < 0.5f;
}

void VoiceControls::updateControlAlpha()
{
    const auto controlAlpha = powerAlpha * (isActive() ? 1.0f : 0.35f);
    gainSlider.setAlpha(controlAlpha);
    selector->setAlpha(controlAlpha);
    bypassButton.setAlpha(powerAlpha);
}

void VoiceControls::configureGainSlider(RotaryLookAndFeel& rotaryLookAndFeel,
                                        juce::Colour voiceColour)
{
    configureRotarySlider(gainSlider, rotaryLookAndFeel, " dB", voiceColour);
}
}
