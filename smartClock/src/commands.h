#pragma once
#include <Arduino.h>

// Parses and executes one line of the serial command protocol (see docs/serial_commands.md).
// Returns an acknowledgement string to send back to the caller.
String processCommand(const String &line);
