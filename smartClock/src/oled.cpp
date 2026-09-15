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

// simple base64 decode used only here
static int base64DecodeLocal(const String &input, uint8_t *out, size_t outMax) {
  auto valOf = [](char c)->int {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return 26 + (c - 'a');
    if (c >= '0' && c <= '9') return 52 + (c - '0');
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
  };
  int outLen = 0;
  int val = 0, valb = -8;
  for (size_t i = 0; i < input.length(); ++i) {
    char c = input[i];
    if (c == '=') break;
    int d = valOf(c);
    if (d == -1) continue;
    val = (val << 6) + d;
    valb += 6;
    if (valb >= 0) {
      if ((size_t)outLen >= outMax) return -1;
      out[outLen++] = (uint8_t)((val >> valb) & 0xFF);
      valb -= 8;
    }
  }
  return outLen;
}

void oledHandleCommand(JsonDocument &doc) {
  if (!doc.containsKey("data")) { publishAck(nullptr, "error", "missing data"); return; }
  const char* b64 = doc["data"];
  static uint8_t bmp[128 * 64 / 8];
  int decoded = base64DecodeLocal(String(b64), bmp, sizeof(bmp));
  if (decoded != (int)sizeof(bmp)) {
    publishAck(nullptr, "error", "invalid bitmap size");
    return;
  }
  u8g2.clearBuffer();
  u8g2.drawXBMP(0, 0, 128, 64, bmp);
  u8g2.sendBuffer();
  publishAck(nullptr, "ok", nullptr);
}
