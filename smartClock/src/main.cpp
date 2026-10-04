#include <Arduino.h>
#include <FastLED.h>
#include "oled.h"
#include "ledarray.h"
#include "buzzer.h"
#include "commands.h"
#include "settings.h"

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
  settingsInit();
  oledInit();
  ledInit();
  applyStoredLeds();
  buzzerInit();
  buzzerPlayAlert();
  applyStoredEmoji();

}

void loop() {
  buzzerUpdate();
  pollSerialCommands();
}
