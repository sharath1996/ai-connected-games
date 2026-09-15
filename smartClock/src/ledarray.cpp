#include "ledarray.h"
#include <FastLED.h>
#include "netmgr.h"

#define PIN_WS2812B 15
#define NUM_PIXELS  10
static CRGB leds[NUM_PIXELS];

void ledInit() {
  FastLED.addLeds<WS2812B, PIN_WS2812B, GRB>(leds, NUM_PIXELS);
  fill_solid(leds, NUM_PIXELS, CRGB::Black);
  FastLED.show();
}

void ledHandleCommand(JsonDocument &doc) {
  if (!doc.containsKey("data")) { publishAck(nullptr, "error", "missing data"); return; }
  JsonArray arr = doc["data"].as<JsonArray>();
  int i = 0;
  for (JsonVariant v : arr) {
    if (i >= NUM_PIXELS) break;
    if (!v.is<JsonArray>()) continue;
    JsonArray rgb = v.as<JsonArray>();
    int r = rgb[0] | 0;
    int g = rgb[1] | 0;
    int b = rgb[2] | 0;
    leds[i] = CRGB(r, g, b);
    ++i;
  }
  if (doc.containsKey("brightness")) {
    int br = doc["brightness"];
    FastLED.setBrightness(constrain(br, 0, 255));
  } else {
    FastLED.setBrightness(255);
  }
  FastLED.show();
  publishAck(nullptr, "ok", nullptr);
}
