#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FastLED.h>
#include <WiFi.h>
#include <time.h>

#include "secrets.h"

// ============================================================
// Hardware & Display Settings
// ============================================================
constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr int OLED_SDA = 21;
constexpr int OLED_SCL = 22;
constexpr uint8_t OLED_ADDRESS = 0x3C;

// Timezone offset (GMT + 5:30 = 19800 seconds)
constexpr long GMT_OFFSET_SEC = 19800;
constexpr int DAYLIGHT_OFFSET_SEC = 0;

// ============================================================
// FastLED & Alarm Settings
// ============================================================
#define PIN_WS2812B 15
#define NUM_PIXELS  10

// Set your alarm time here (24-hour format)
constexpr int ALARM_HOUR = 7;
constexpr int ALARM_MINUTE = 0;

constexpr unsigned long SUNRISE_DURATION = 5000;  // 5 seconds
constexpr unsigned long HOLD_DURATION    = 2000;  // 2 seconds stay bright
constexpr unsigned long SUNSET_DURATION  = 5000;  // 5 seconds

// Objects & LED buffer
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
CRGB leds[NUM_PIXELS];

// Key colors for smooth FastLED blending
const CRGB COLOR_NIGHT   = CRGB::Black;
const CRGB COLOR_DAWN    = CRGB(180, 40, 5);      // Warm Red/Orange
const CRGB COLOR_DAY     = CRGB(255, 220, 80);    // Bright Golden Daylight
const CRGB COLOR_DUSK    = CRGB(180, 30, 60);     // Sunset Purple/Dusk

// Alarm animation state
enum AlarmState {
  IDLE,
  SUNRISE,
  HOLD,
  SUNSET
};

AlarmState alarmState = IDLE;
unsigned long stateStartTime = 0;
int lastTriggeredMinute = -1;

unsigned long lastDisplayUpdate = 0;
unsigned long lastLedUpdate = 0;

// ============================================================
// Non-blocking Alarm Animation
// ============================================================
void updateAlarm(const struct tm &timeInfo) {
  unsigned long now = millis();

  // Check if alarm should trigger (once per minute)
  if (alarmState == IDLE) {
    if (timeInfo.tm_hour == ALARM_HOUR && timeInfo.tm_min == ALARM_MINUTE && lastTriggeredMinute != timeInfo.tm_min) {
      alarmState = SUNRISE;
      stateStartTime = now;
      lastTriggeredMinute = timeInfo.tm_min;
      Serial.println("Alarm triggered! Starting Sunrise...");
    }
  }

  // Reset triggered flag when the minute passes
  if (timeInfo.tm_min != lastTriggeredMinute) {
    lastTriggeredMinute = -1;
  }

  // Update LED colors every 20 ms
  if (now - lastLedUpdate < 20) {
    return;
  }
  lastLedUpdate = now;

  if (alarmState == SUNRISE) {
    unsigned long elapsed = now - stateStartTime;
    if (elapsed >= SUNRISE_DURATION) {
      fill_solid(leds, NUM_PIXELS, COLOR_DAY);
      FastLED.show();
      alarmState = HOLD;
      stateStartTime = now;
      Serial.println("Sunrise complete. Holding light...");
    } else {
      float progress = (float)elapsed / (float)SUNRISE_DURATION;
      CRGB currentColor;
      // Sunrise: Black -> Dawn (0-40%) -> Daylight (40-100%)
      if (progress < 0.4f) {
        uint8_t blendAmount = (uint8_t)((progress / 0.4f) * 255.0f);
        currentColor = blend(COLOR_NIGHT, COLOR_DAWN, blendAmount);
      } else {
        uint8_t blendAmount = (uint8_t)(((progress - 0.4f) / 0.6f) * 255.0f);
        currentColor = blend(COLOR_DAWN, COLOR_DAY, blendAmount);
      }
      fill_solid(leds, NUM_PIXELS, currentColor);
      FastLED.show();
    }
  } 
  else if (alarmState == HOLD) {
    if (now - stateStartTime >= HOLD_DURATION) {
      alarmState = SUNSET;
      stateStartTime = now;
      Serial.println("Starting Sunset...");
    }
  } 
  else if (alarmState == SUNSET) {
    unsigned long elapsed = now - stateStartTime;
    if (elapsed >= SUNSET_DURATION) {
      fill_solid(leds, NUM_PIXELS, COLOR_NIGHT);
      FastLED.show();
      alarmState = IDLE;
      Serial.println("Sunset complete. Alarm finished.");
    } else {
      float progress = (float)elapsed / (float)SUNSET_DURATION;
      CRGB currentColor;
      // Sunset: Daylight -> Dusk (0-50%) -> Black (50-100%)
      if (progress < 0.5f) {
        uint8_t blendAmount = (uint8_t)((progress / 0.5f) * 255.0f);
        currentColor = blend(COLOR_DAY, COLOR_DUSK, blendAmount);
      } else {
        uint8_t blendAmount = (uint8_t)(((progress - 0.5f) / 0.5f) * 255.0f);
        currentColor = blend(COLOR_DUSK, COLOR_NIGHT, blendAmount);
      }
      fill_solid(leds, NUM_PIXELS, currentColor);
      FastLED.show();
    }
  }
}

// Non-blocking OLED display update
void updateDisplay(const struct tm &timeInfo) {
  display.clearDisplay();

  // Format time: HH:MM:SS
  char timeStr[9];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", 
           timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);

  // Format date: Day, DD Mon YYYY (e.g. "Sun, 13 Sep 2026")
  char dateStr[20];
  strftime(dateStr, sizeof(dateStr), "%a, %d %b %Y", &timeInfo);

  // Display Time (centered)
  display.setTextSize(2);
  display.setCursor(16, 14);
  display.print(timeStr);

  // Display Date (centered)
  display.setTextSize(1);
  display.setCursor(16, 42);
  display.print(dateStr);

  display.display();
}

// ============================================================
// Setup
// ============================================================
void setup() {
  Serial.begin(115200);

  // Initialize FastLED
  FastLED.addLeds<WS2812B, PIN_WS2812B, GRB>(leds, NUM_PIXELS);
  fill_solid(leds, NUM_PIXELS, CRGB::Black);
  FastLED.show();

  // Initialize OLED display
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED initialization failed!");
    while (true) delay(1000);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(15, 25);
  display.println("Connecting WiFi...");
  display.display();

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // Initialize NTP time sync
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, "pool.ntp.org");
  
  display.clearDisplay();
  display.setCursor(20, 25);
  display.println("Syncing Time...");
  display.display();

  struct tm timeInfo;
  while (!getLocalTime(&timeInfo)) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nTime synced!");
}

// ============================================================
// Loop (Completely Non-blocking)
// ============================================================
void loop() {
  struct tm timeInfo;
  if (getLocalTime(&timeInfo, 10)) {
    // Non-blocking alarm check and LED updates
    updateAlarm(timeInfo);

    // Update screen every 200 ms for smooth ticking
    unsigned long now = millis();
    if (now - lastDisplayUpdate >= 200) {
      lastDisplayUpdate = now;
      updateDisplay(timeInfo);
    }
  }
}
