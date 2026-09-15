#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

void ledInit();
void ledHandleCommand(JsonDocument &doc);
