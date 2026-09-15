# Pin Connections

| Signal | ESP32 Pin / Connector | Connection | Notes |
|---|---:|---|---|
| WS2812B Data | GPIO15 | Data input of WS2812B LED strip | `NUM_PIXELS = 10`. Use a logic-level shifter if powering LEDs with 5V; include a common GND. |
| OLED SDA (I2C) | GPIO21 | OLED SDA | SSD1306 I2C address `0x3C`. |
| OLED SCL (I2C) | GPIO22 | OLED SCL |  |
| OLED VCC | 3.3V (or 5V depending on module) | OLED VCC | Check your OLED module's voltage requirement before connecting. |
| OLED GND | GND | OLED GND |  |
| USB Serial | USB / UART-to-USB | Serial Monitor | `Serial.begin(115200)` used for interaction and alarm input. |
| Wi‑Fi | internal | N/A | Uses ESP32 on‑chip Wi‑Fi (no external pins required). |
| Power (LEDs) | 5V or external supply | LED VCC | Provide adequate current for the LED strip; use a large decoupling capacitor near the strip. |

## Notes

- Drive the WS2812B data line from a 3.3V-tolerant data pin; when powering the LEDs with 5V, a level shifter or resistor and proper signal conditioning is recommended.
- Keep a common ground between the ESP32 and the LED power supply.
- Use a 1000 µF electrolytic capacitor across the LED power rails to prevent voltage spikes when the strip draws current.
- Confirm your OLED module voltage (many SSD1306 modules accept 3.3V or 5V). If uncertain, power from 3.3V to be safe.
