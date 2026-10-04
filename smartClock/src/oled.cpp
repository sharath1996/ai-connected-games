#include "oled.h"
#include <U8g2lib.h>
#include <Wire.h>
#include <string.h>

constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
// GPIO5/GPIO6: free pins on the C3 Super Mini, clear of strapping/boot/USB pins.
constexpr int OLED_SDA = 5;
constexpr int OLED_SCL = 6;

static U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void oledInit() {
  Wire.begin(OLED_SDA, OLED_SCL);
  Wire.setClock(400000);
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  const char *msg = "Awe...";
  int msgW = u8g2.getUTF8Width(msg);
  u8g2.drawStr((SCREEN_WIDTH - msgW) / 2, 28, msg);
  u8g2.sendBuffer();
}

void displayEmoji(const char *emoji) {
  if (emoji == nullptr || emoji[0] == '\0') {
    return;
  }

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_unifont_t_symbols);
  u8g2.drawUTF8(0, 32, emoji);

  static uint8_t glyphBuffer[SCREEN_WIDTH * SCREEN_HEIGHT / 8];
  memcpy(glyphBuffer, u8g2.getBufferPtr(), sizeof(glyphBuffer));

  int minX = SCREEN_WIDTH;
  int minY = SCREEN_HEIGHT;
  int maxX = -1;
  int maxY = -1;
  for (int y = 0; y < SCREEN_HEIGHT; ++y) {
    for (int x = 0; x < SCREEN_WIDTH; ++x) {
      size_t index = (y / 8) * SCREEN_WIDTH + x;
      if ((glyphBuffer[index] & (1U << (y % 8))) != 0) {
        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
      }
    }
  }

  if (maxX < minX || maxY < minY) {
    u8g2.sendBuffer();
    return;
  }

  int glyphWidth = maxX - minX + 1;
  int glyphHeight = maxY - minY + 1;
  int scaledWidth;
  int scaledHeight;
  if ((SCREEN_WIDTH - 8) * glyphHeight <= (SCREEN_HEIGHT - 8) * glyphWidth) {
    scaledWidth = SCREEN_WIDTH - 8;
    scaledHeight = glyphHeight * scaledWidth / glyphWidth;
  } else {
    scaledHeight = SCREEN_HEIGHT - 8;
    scaledWidth = glyphWidth * scaledHeight / glyphHeight;
  }
  int offsetX = (SCREEN_WIDTH - scaledWidth) / 2;
  int offsetY = (SCREEN_HEIGHT - scaledHeight) / 2;

  u8g2.clearBuffer();
  for (int y = 0; y < scaledHeight; ++y) {
    for (int x = 0; x < scaledWidth; ++x) {
      int sourceX = minX + x * glyphWidth / scaledWidth;
      int sourceY = minY + y * glyphHeight / scaledHeight;
      size_t index = (sourceY / 8) * SCREEN_WIDTH + sourceX;
      if ((glyphBuffer[index] & (1U << (sourceY % 8))) != 0) {
        u8g2.drawPixel(offsetX + x, offsetY + y);
      }
    }
  }
  u8g2.sendBuffer();
}


