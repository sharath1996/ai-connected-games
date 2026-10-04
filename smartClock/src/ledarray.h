#pragma once
#include <Arduino.h>
#include <FastLED.h>

void ledInit();
void defaultLEDColor(CRGB color);
void setLEDcolor(int index, CRGB color);
void setLEDBrightness(uint8_t brightness);
void setLEDBrightnessAutomation(CRGB color, uint8_t brightnessStart, uint8_t brightnessEnd, uint8_t durationInSeconds);
void updateLEDBrightnessAutomation();



