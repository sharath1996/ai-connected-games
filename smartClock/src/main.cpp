#include <Arduino.h>
#include <FastLED.h>
#include "oled.h"
#include "ledarray.h"
#include "buzzer.h"
#include "switch.h"

void setup() {
  Serial.begin(115200);
  oledInit();
  ledInit();
  defaultLEDColor(CRGB::Green);
  setLEDBrightnessAutomation(CRGB::Green, 16, 160, 5);
  buzzerInit();
  switchInit();
  displayEmoji("♥");

}

void loop() {
  if (switchWasPressed()) {
    buzzerPlayAlert();
  }
  buzzerUpdate();
  updateLEDBrightnessAutomation();
}
