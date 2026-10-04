#include "settings.h"
#include "ledarray.h"
#include "oled.h"
#include <Preferences.h>

namespace {
const char *NS = "smartclock";  // Preferences namespace (max 15 chars)

Preferences prefs;
ClockSettings settings;
bool ready = false;
}

void settingsInit() {
  ready = prefs.begin(NS, /*readOnly=*/true);
  if (ready) {
    settings.color = CRGB(prefs.getUInt("rgb", CRGB::Green));
    settings.brightness = prefs.getUChar("bright", 0);
    settings.emoji = prefs.getString("emoji", "♥");
    prefs.end();
  }
}

void applyStoredLeds() {
  defaultLEDColor(settings.color);
  if (settings.brightness != 0) {
    setLEDBrightness(settings.brightness);
  }
}

void applyStoredEmoji() {
  if (settings.emoji.length() > 0) {
    displayEmoji(settings.emoji.c_str());
  }
}

void saveDefaultColor(CRGB color) {
  settings.color = color;
  if (!ready) return;
  prefs.begin(NS, false);
  prefs.putUInt("rgb", static_cast<uint32_t>(color));
  prefs.end();
}

void saveDefaultBrightness(uint8_t brightness) {
  settings.brightness = brightness;
  if (!ready) return;
  prefs.begin(NS, false);
  prefs.putUChar("bright", brightness);
  prefs.end();
}

void saveDefaultEmoji(const String &emoji) {
  settings.emoji = emoji;
  if (!ready) return;
  prefs.begin(NS, false);
  prefs.putString("emoji", emoji);
  prefs.end();
}

const ClockSettings &currentSettings() {
  return settings;
}
