#include "buzzer.h"

namespace {
constexpr uint8_t BUZZER_PIN = 7;
constexpr unsigned int NOTES[] = {880, 1175, 1568};
constexpr unsigned long NOTE_DURATION_MS = 140;
constexpr unsigned long NOTE_GAP_MS = 70;

uint8_t noteIndex = 0;
bool alertActive = false;
bool notePlaying = false;
unsigned long transitionAt = 0;
}

void buzzerInit() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  noTone(BUZZER_PIN);
}

void buzzerPlayAlert() {
  noTone(BUZZER_PIN);
  noteIndex = 0;
  alertActive = true;
  notePlaying = false;
  transitionAt = millis();
}

void buzzerUpdate() {
  if (!alertActive || static_cast<long>(millis() - transitionAt) < 0) {
    return;
  }

  if (notePlaying) {
    noTone(BUZZER_PIN);
    notePlaying = false;
    ++noteIndex;
    if (noteIndex >= sizeof(NOTES) / sizeof(NOTES[0])) {
      alertActive = false;
      return;
    }
    transitionAt = millis() + NOTE_GAP_MS;
    return;
  }

  tone(BUZZER_PIN, NOTES[noteIndex]);
  notePlaying = true;
  transitionAt = millis() + NOTE_DURATION_MS;
}

void buzzerStop() {
  noTone(BUZZER_PIN);
  digitalWrite(BUZZER_PIN, LOW);
  alertActive = false;
  notePlaying = false;
}