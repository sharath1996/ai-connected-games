#include "ledarray.h"
// GPIO20: right side of C3 Super Mini, free pin when using native USB CDC
#define PIN_WS2812B 20
#define NUM_PIXELS  5
static CRGB leds[NUM_PIXELS];

namespace {
bool brightnessAutomationActive = false;
uint8_t brightnessStart = 0;
uint8_t brightnessEnd = 0;
uint8_t appliedBrightness = 0;
uint32_t brightnessDurationMs = 0;
uint32_t brightnessStartedAt = 0;
CRGB currentColor = CRGB::Green;
}

void ledInit() {
  FastLED.addLeds<WS2812B, PIN_WS2812B, GRB>(leds, NUM_PIXELS);
  fill_solid(leds, NUM_PIXELS, CRGB::Green);
  FastLED.show();
}

void defaultLEDColor(CRGB color){
  currentColor = color;
  for (int i = 0; i < NUM_PIXELS; ++i) {
    setLEDcolor(i, color);
    delay(10);
  }
}

void refreshLED() {
  defaultLEDColor(currentColor);
}

void setLEDcolor(int index, CRGB color) {
  if (index < 0 || index >= NUM_PIXELS) return;
  leds[index] = color;
  FastLED.show();
}

void setLEDBrightness(uint8_t brightness) {
  brightnessAutomationActive = false;
  appliedBrightness = brightness;
  FastLED.setBrightness(brightness);
  FastLED.show();
}

void setLEDBrightnessAutomation(CRGB color, uint8_t start, uint8_t end, uint8_t durationInSeconds) {
  fill_solid(leds, NUM_PIXELS, color);
  brightnessStart = start;
  brightnessEnd = end;
  brightnessDurationMs = static_cast<uint32_t>(durationInSeconds) * 1000;
  brightnessStartedAt = millis();

  if (brightnessDurationMs == 0) {
    brightnessAutomationActive = false;
    appliedBrightness = brightnessEnd;
  } else {
    brightnessAutomationActive = true;
    appliedBrightness = brightnessStart;
  }
  FastLED.setBrightness(appliedBrightness);
  FastLED.show();
}

void updateLEDBrightnessAutomation() {
  if (!brightnessAutomationActive) return;

  uint32_t elapsed = millis() - brightnessStartedAt;
  uint8_t nextBrightness;
  if (elapsed >= brightnessDurationMs) {
    nextBrightness = brightnessEnd;
    brightnessAutomationActive = false;
  } else {
    int16_t brightnessDelta = static_cast<int16_t>(brightnessEnd) - brightnessStart;
    int32_t interpolated = brightnessStart +
        static_cast<int32_t>(brightnessDelta) * elapsed / brightnessDurationMs;
    nextBrightness = static_cast<uint8_t>(interpolated);
  }

  if (nextBrightness == appliedBrightness) return;
  appliedBrightness = nextBrightness;
  FastLED.setBrightness(appliedBrightness);
  FastLED.show();
}