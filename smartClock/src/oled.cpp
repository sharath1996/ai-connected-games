#include "oled.h"
#include <U8g2lib.h>
#include <Wire.h>
#include "netmgr.h"

constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr int OLED_SDA = 21;
constexpr int OLED_SCL = 22;

static U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void oledInit() {
  Wire.begin(OLED_SDA, OLED_SCL);
  Wire.setClock(400000);
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  const char *msg = "Waiting MQTT...";
  int msgW = u8g2.getUTF8Width(msg);
  u8g2.drawStr((SCREEN_WIDTH - msgW) / 2, 28, msg);
  u8g2.sendBuffer();
}

static const int TOTAL_PIXELS = SCREEN_WIDTH * SCREEN_HEIGHT; // 8192
static const int PACKED_SIZE = TOTAL_PIXELS / 8; // 1024

// "data" is a plain string of '0'/'1' chars, one per pixel, row-major (no base64/BMP)
void oledHandleCommand(JsonDocument &doc) {
  if (!doc.containsKey("data")) { Serial.println("OLED: missing data"); publishAck(nullptr, "error", "missing data"); return; }
  const char* pixels = doc["data"];
  size_t len = strlen(pixels);
  if (len != (size_t)TOTAL_PIXELS) {
    Serial.printf("OLED: expected %d pixel chars, got %u\n", TOTAL_PIXELS, (unsigned)len);
    publishAck(nullptr, "error", "expected 128*64 pixel chars");
    return;
  }

  static uint8_t bmp[PACKED_SIZE];
  memset(bmp, 0, sizeof(bmp));
  for (int i = 0; i < TOTAL_PIXELS; ++i) {
    char c = pixels[i];
    if (c == '1') {
      bmp[i / 8] |= (1 << (i % 8)); // LSB-first per byte, matching XBM row order
    } else if (c != '0') {
      publishAck(nullptr, "error", "invalid pixel char, expected 0/1");
      return;
    }
  }

  u8g2.clearBuffer();
  u8g2.drawXBMP(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, bmp);
  u8g2.sendBuffer();
  Serial.println("OLED: updated");
  publishAck(nullptr, "ok", nullptr);
}

