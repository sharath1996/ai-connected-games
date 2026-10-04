#pragma once
#include <Arduino.h>
#include <FastLED.h>

void ledInit();
void defaultLEDColor(CRGB color);

// Re-sends the last color set via defaultLEDColor(). Call periodically to keep
// pixels lit when the first pixel's data input is marginal.
void refreshLED();
void setLEDcolor(int index, CRGB color);
void setLEDBrightness(uint8_t brightness);
void setLEDBrightnessAutomation(CRGB color, uint8_t brightnessStart, uint8_t brightnessEnd, uint8_t durationInSeconds);
void updateLEDBrightnessAutomation();



