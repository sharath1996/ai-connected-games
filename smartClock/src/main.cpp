#include <Arduino.h>
#include <FastLED.h>
#include "oled.h"
#include "ledarray.h"
#include "buzzer.h"
#include "commands.h"

namespace {
String serialLineBuffer;

void pollSerialCommands() {
  while (Serial.available() > 0) {
    char c = static_cast<char>(Serial.read());
    if (c == '\n' || c == '\r') {
      if (serialLineBuffer.length() > 0) {
        String ack = processCommand(serialLineBuffer);
        if (ack.length() > 0) Serial.println(ack);
        serialLineBuffer = "";
      }
    } else {
      serialLineBuffer += c;
    }
  }
}
}  // namespace

void setup() {
  Serial.begin(115200);
  oledInit();
  ledInit();
  defaultLEDColor(CRGB::Green);
  // setLEDBrightness(160);
  buzzerInit();
  displayEmoji("♥");

}

void loop() {

  buzzerUpdate();
  pollSerialCommands();

  // Re-transmit the frame periodically: masks a marginal first-pixel latch.
  static uint32_t lastRefreshAt = 0;
  if (millis() - lastRefreshAt >= 500) {
    lastRefreshAt = millis();
    refreshLED();
  }

}