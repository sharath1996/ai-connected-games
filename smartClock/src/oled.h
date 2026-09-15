#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

void oledInit();
void oledHandleCommand(JsonDocument &doc);
