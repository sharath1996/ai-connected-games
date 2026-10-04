#include "commands.h"
#include "ledarray.h"
#include "buzzer.h"
#include "oled.h"
#include "settings.h"
#include <string.h>

namespace {

int argToInt(const char *arg) {
  return arg != nullptr ? atoi(arg) : 0;
}

bool handleTrigger(char *savePtr) {
  char *rArg = strtok_r(nullptr, " ,)", &savePtr);
  char *gArg = strtok_r(nullptr, " ,)", &savePtr);
  char *bArg = strtok_r(nullptr, " ,)", &savePtr);
  char *emojiArg = strtok_r(nullptr, " ,)", &savePtr);
  char *soundArg = strtok_r(nullptr, " ,)", &savePtr);
  if (!rArg || !gArg || !bArg || !emojiArg || !soundArg) return false;

  defaultLEDColor(CRGB(argToInt(rArg), argToInt(gArg), argToInt(bArg)));
  displayEmoji(emojiArg);
  saveDefaultEmoji(String(emojiArg));  // remember so boot shows the latest
  if (argToInt(soundArg) != 0) {
    buzzerPlayAlert();
  } else {
    buzzerStop();
  }
  return true;
}

// PERSIST_LED R G B [BRIGHTNESS] — save as power-on default (does not light the LEDs).
bool handlePersistLed(char *savePtr) {
  char *rArg = strtok_r(nullptr, " ,)", &savePtr);
  char *gArg = strtok_r(nullptr, " ,)", &savePtr);
  char *bArg = strtok_r(nullptr, " ,)", &savePtr);
  char *brightArg = strtok_r(nullptr, " ,)", &savePtr);  // optional
  if (!rArg || !gArg || !bArg) return false;

  saveDefaultColor(CRGB(argToInt(rArg), argToInt(gArg), argToInt(bArg)));
  if (brightArg) {
    int brightness = argToInt(brightArg);
    if (brightness < 1 || brightness > 255) return false;
    saveDefaultBrightness(static_cast<uint8_t>(brightness));
  }
  return true;
}

// PERSIST_EMOJI X — save as power-on default OLED symbol (does not redraw now).
bool handlePersistEmoji(char *savePtr) {
  char *emojiArg = strtok_r(nullptr, " ,)", &savePtr);
  if (!emojiArg) return false;
  saveDefaultEmoji(String(emojiArg));
  return true;
}

}  // namespace

String processCommand(const String &rawLine) {
  String line = rawLine;
  line.trim();
  if (line.length() == 0) return "";

  char buf[160];
  line.toCharArray(buf, sizeof(buf));

  char *savePtr = nullptr;
  char *cmd = strtok_r(buf, " (", &savePtr);
  if (cmd == nullptr) return "ERR EMPTY";

  bool ok = false;
  if (strcmp(cmd, "TRIGGER") == 0) {
    ok = handleTrigger(savePtr);
  } else if (strcmp(cmd, "PERSIST_LED") == 0) {
    ok = handlePersistLed(savePtr);
  } else if (strcmp(cmd, "PERSIST_EMOJI") == 0) {
    ok = handlePersistEmoji(savePtr);
  } else {
    return "ERR UNKNOWN_CMD " + String(cmd);
  }

  return (ok ? "ACK " : "ERR BAD_ARGS ") + String(cmd);
}
