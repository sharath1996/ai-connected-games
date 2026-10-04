#pragma once
#include <Arduino.h>
#include <FastLED.h>

// Persistent SmartClock settings stored in flash via Preferences (ESP32's
// wear-leveled NVS — the modern replacement for emulated EEPROM).
struct ClockSettings {
  CRGB color = CRGB::Green;   // default LED color
  uint8_t brightness = 0;     // 0 = unset (FastLED default 255)
  String emoji = "♥";          // default OLED symbol
};

// Loads settings from flash into RAM. Call once before ledInit().
void settingsInit();

// Applies stored color/brightness to the LED strip. Call after ledInit().
void applyStoredLeds();

// Shows the stored emoji on the OLED. Call after oledInit().
void applyStoredEmoji();

// Persist new defaults (each writes to flash — flash has ~100k write cycles,
// so these are for occasional "save as default" use, not per-second updates).
void saveDefaultColor(CRGB color);
void saveDefaultBrightness(uint8_t brightness);
void saveDefaultEmoji(const String &emoji);

// The currently loaded settings.
const ClockSettings &currentSettings();
