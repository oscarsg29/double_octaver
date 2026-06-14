#pragma once

namespace double_octaver::gui::layout
{
namespace editor
{
constexpr int width = 330;
constexpr int height = 380;
}

namespace header
{
constexpr int height = 42;
constexpr int titleX = 14;
constexpr int titleY = 10;
constexpr int titleWidth = 130;
constexpr int titleHeight = 20;

constexpr float powerLedRightMargin = 83.0f;
constexpr float powerLedY = 18.0f;
constexpr float powerLedSize = 6.0f;

constexpr float screw1X = 14.0f;
constexpr float screw2X = 112.0f;
constexpr float screw3X = 218.0f;
constexpr float screwRightMargin = 22.0f;
constexpr float screwY = 51.0f;
constexpr float screwSize = 8.0f;
constexpr float screwSlotY = 55.0f;
constexpr float screwSlotInset = 2.0f;
}

namespace voices
{
constexpr int width = 94;
constexpr int height = 238;
constexpr int firstX = 15;
constexpr int secondX = 110;
constexpr int y = 106;

constexpr int sectionLabelX = 18;
constexpr int sectionLabelWidth = 210;
constexpr float ruleStartX = 18.0f;
constexpr float ruleEndX = 204.0f;
}

namespace voiceControl
{
constexpr int knobX = 9;
constexpr int knobY = 0;
constexpr int gainKnobHeight = 94;

constexpr int selectorY = 98;
constexpr int selectorHeight = 118;

constexpr int bypassX = 15;
constexpr int bypassY = 220;
constexpr int bypassWidth = 64;
constexpr int bypassHeight = 18;
}

namespace output
{
constexpr int x = 236;
constexpr int mixY = 106;
constexpr int masterY = 231;
constexpr int knobHeight = 92;

constexpr int sectionLabelWidth = 74;
constexpr float ruleStartX = 236.0f;
constexpr int ruleRightMargin = 20;
}

namespace controls
{
constexpr int knobWidth = 76;
constexpr int labelY = 96;
constexpr int masterLabelY = 221;
constexpr int labelHeight = 12;
}

namespace powerButton
{
constexpr int rightMargin = 61;
constexpr int y = 12;
constexpr int width = 38;
constexpr int height = 18;
}

namespace sections
{
constexpr int labelY = 70;
constexpr int labelHeight = 16;
constexpr int ruleY = 90;
}

namespace divider
{
constexpr float gradientX = 219.0f;
constexpr float gradientTopY = 65.0f;
constexpr float gradientBottomY = 150.0f;
constexpr int x = 220;
constexpr float topY = 65.0f;
constexpr int bottomMargin = 42;
}

namespace footer
{
constexpr int height = 30;
constexpr int textX = 14;
constexpr int textBottomMargin = 22;
constexpr int textWidth = 126;
constexpr int textHeight = 14;
}

namespace status
{
constexpr float dryX = 180.0f;
constexpr float voice1X = 217.0f;
constexpr float voice2X = 250.0f;
constexpr float outputX = 283.0f;
constexpr int dotBottomMargin = 15;
}
}
