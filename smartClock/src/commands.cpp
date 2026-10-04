#include "commands.h"
#include "ledarray.h"
#include "buzzer.h"
#include "oled.h"
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
  if (argToInt(soundArg) != 0) {
    buzzerPlayAlert();
  } else {
    buzzerStop();
  }
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
  } else {
    return "ERR UNKNOWN_CMD " + String(cmd);
  }

  return (ok ? "ACK " : "ERR BAD_ARGS ") + String(cmd);
}
