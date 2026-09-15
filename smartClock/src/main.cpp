#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <U8g2lib.h>
#include <FastLED.h>
#include <WiFi.h>
#include <time.h>

#include "secrets.h"

// ============================================================
// Hardware & Display Settings
// ============================================================
constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64; // If your module is 128x32 change this to 32
constexpr int OLED_SDA = 21;
constexpr int OLED_SCL = 22;
uint8_t OLED_ADDRESS =0x3c; // try 0x3C or 0x3D; scanner will list detected addresses

// Timezone offset (GMT + 5:30 = 19800 seconds)
constexpr long GMT_OFFSET_SEC = 19800;
constexpr int DAYLIGHT_OFFSET_SEC = 0;

// ============================================================
// FastLED & Alarm Settings
// ============================================================
#define PIN_WS2812B 15
#define NUM_PIXELS  10

// Alarm time (can be set via Serial console at startup)
int alarmHour = 7;
int alarmMinute = 0;

constexpr unsigned long SUNRISE_DURATION = 5000;  // 5 seconds
constexpr unsigned long HOLD_DURATION    = 2000;  // 2 seconds stay bright
constexpr unsigned long SUNSET_DURATION  = 5000;  // 5 seconds

// Objects & LED buffer
// Use U8g2 with SH1106 128x64 (works with many 1.3" modules).
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
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
    if (timeInfo.tm_hour == alarmHour && timeInfo.tm_min == alarmMinute && lastTriggeredMinute != timeInfo.tm_min) {
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

// Non-blocking OLED display update using u8g2
void updateDisplay(const struct tm &timeInfo) {
  // Format time: HH:MM:SS
  char timeStr[9];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", 
           timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);

  // Format date: Day, DD Mon YYYY (e.g. "Sun, 13 Sep 2026")
  char dateStr[20];
  strftime(dateStr, sizeof(dateStr), "%a, %d %b %Y", &timeInfo);

  u8g2.clearBuffer();

  // Time (large font, centered)
  u8g2.setFont(u8g2_font_ncenB24_tr);
  int timeW = u8g2.getUTF8Width(timeStr);
  u8g2.drawStr((SCREEN_WIDTH - timeW) / 2, 32, timeStr);

  // Date (smaller font, centered near bottom)
  u8g2.setFont(u8g2_font_6x12_tr);
  int dateW = u8g2.getUTF8Width(dateStr);
  u8g2.drawStr((SCREEN_WIDTH - dateW) / 2, 58, dateStr);

  u8g2.sendBuffer();
}

// Non-blocking Serial alarm input
String serialBuffer = "";

// Parse and set alarm from an input string like "HH:MM" or "HHMM" or "HH".
void setAlarmFromString(const String &inputRaw) {
  String input = inputRaw;
  input.trim();
  if (input.length() == 0) {
    Serial.println("Keeping current alarm time.");
    return;
  }

  int h = -1, m = -1;
  int sep = input.indexOf(':');
  if (sep > 0) {
    String hs = input.substring(0, sep);
    String ms = input.substring(sep + 1);
    h = hs.toInt();
    m = ms.toInt();
  } else {
    if (input.length() <= 2) {
      h = input.toInt();
      m = 0;
    } else if (input.length() == 4) {
      h = input.substring(0, 2).toInt();
      m = input.substring(2).toInt();
    }
  }

  if (h >= 0 && h <= 23 && m >= 0 && m <= 59) {
    alarmHour = h;
    alarmMinute = m;
    Serial.print("Alarm set to ");
    if (alarmHour < 10) Serial.print('0');
    Serial.print(alarmHour);
    Serial.print(':');
    if (alarmMinute < 10) Serial.print('0');
    Serial.println(alarmMinute);
  } else {
    Serial.println("Invalid format; keep current alarm time.");
  }
}

// Call regularly from loop() to collect Serial input non-blocking.
void processSerialInput() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      if (serialBuffer.length() > 0) {
        setAlarmFromString(serialBuffer);
        serialBuffer = "";
      }
      Serial.print("> ");
    } else {
      serialBuffer += c;
    }
  }
}

// Print initial non-blocking prompt (no waiting).
void readAlarmFromSerial() {
  Serial.println();
  Serial.print("Current alarm: ");
  if (alarmHour < 10) Serial.print('0');
  Serial.print(alarmHour);
  Serial.print(':');
  if (alarmMinute < 10) Serial.print('0');
  Serial.println(alarmMinute);
  Serial.println("You can set alarm anytime via Serial with HH:MM (24-hour).\nType and press Enter:");
  Serial.print("> ");
}

// ============================================================
// Setup
// ============================================================
void setup() {
  Serial.begin(115200);

#ifdef SSD1306_128_32
  const char* oledType = "SSD1306_128_32";
#else
  const char* oledType = "SSD1306_128_64";
#endif
  Serial.print("Configured display: "); Serial.print(SCREEN_WIDTH); Serial.print("x"); Serial.println(SCREEN_HEIGHT);
  Serial.println("Using U8g2 (SH1106 driver) for OLED");

  // Initialize FastLED
  FastLED.addLeds<WS2812B, PIN_WS2812B, GRB>(leds, NUM_PIXELS);
  fill_solid(leds, NUM_PIXELS, CRGB::Black);
  FastLED.show();

  // Initialize OLED display
  Wire.begin(OLED_SDA, OLED_SCL);
  // Increase I2C speed for reliability
  Wire.setClock(400000);

  // Quick I2C scan to help diagnose OLED address/connection issues
  Serial.println("Scanning I2C bus for devices...");
  bool foundAny = false;
  for (uint8_t addr = 1; addr < 127; ++addr) {
    Wire.beginTransmission(addr);
    uint8_t err = Wire.endTransmission();
    if (err == 0) {
      Serial.print("  Found device at 0x");
      if (addr < 16) Serial.print('0');
      Serial.println(addr, HEX);
      foundAny = true;
    }
  }
  if (!foundAny) {
    Serial.println("  No I2C devices found. Check wiring (SDA/SCL/GND/VCC).\n");
  }
  // Initialize u8g2 (SH1106/compatible). u8g2 uses Wire; Wire already began above.
  u8g2.begin();
  Serial.println("u8g2 initialized");

  // Show initial message on the OLED
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  const char *msg = "Connecting WiFi...";
  int msgW = u8g2.getUTF8Width(msg);
  u8g2.drawStr((SCREEN_WIDTH - msgW) / 2, 28, msg);
  u8g2.sendBuffer();

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
  
  u8g2.clearBuffer();
  const char *syncMsg = "Syncing Time...";
  int syncW = u8g2.getUTF8Width(syncMsg);
  u8g2.drawStr((SCREEN_WIDTH - syncW) / 2, 28, syncMsg);
  u8g2.sendBuffer();

  struct tm timeInfo;
  while (!getLocalTime(&timeInfo)) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nTime synced!");
  readAlarmFromSerial();
}

// ============================================================
// Loop (Completely Non-blocking)
// ============================================================
void loop() {
  struct tm timeInfo;
  // Always process Serial input non-blocking
  processSerialInput();

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
