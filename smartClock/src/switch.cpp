#include "switch.h"

namespace {
constexpr uint8_t SWITCH_PIN = 10;
constexpr unsigned long DEBOUNCE_MS = 35;

bool lastReading = false;
bool stablePressed = false;
unsigned long lastChangeAt = 0;
}

void switchInit() {
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  lastReading = digitalRead(SWITCH_PIN) == LOW;
  stablePressed = lastReading;
  lastChangeAt = millis();
}

bool switchWasPressed() {
  const unsigned long now = millis();
  const bool reading = digitalRead(SWITCH_PIN) == LOW;
  if (reading != lastReading) {
    lastReading = reading;
    lastChangeAt = now;
  }

  if (reading != stablePressed && now - lastChangeAt >= DEBOUNCE_MS) {
    stablePressed = reading;
    return stablePressed;
  }
  return false;
}