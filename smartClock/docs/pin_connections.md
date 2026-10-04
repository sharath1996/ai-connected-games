# Pin Connections

Board: **ESP32-C3 Super Mini** (native USB, 11 usable GPIOs: 0–10).

| Signal | ESP32-C3 Pin | Connection | Notes |
|---|---:|---|---|
| WS2812B Data | GPIO20 | Data input of WS2812B LED strip | `NUM_PIXELS = 5`. Use a logic-level shifter if powering LEDs with 5V; include a common GND. |
| Buzzer signal | GPIO7 | Passive buzzer signal (+) | Plays a three-note alert using `tone()`. Connect buzzer GND to board GND. Use a transistor driver if the buzzer exceeds a GPIO's current rating. |
| Push switch | GPIO10 | One switch terminal | Other switch terminal to GND; uses the internal pull-up. Pressing it starts the buzzer alert. |
| OLED SDA (I2C) | GPIO5 | OLED SDA | SSD1306/SH1106 I2C address `0x3C`. |
| OLED SCL (I2C) | GPIO6 | OLED SCL |  |
| OLED VCC | 3.3V (or 5V depending on module) | OLED VCC | Check your OLED module's voltage requirement before connecting. |
| OLED GND | GND | OLED GND |  |
| USB Serial | Onboard USB-C (native USB) | Serial Monitor | `Serial.begin(115200)`; requires `ARDUINO_USB_CDC_ON_BOOT=1` build flag (already set in `platformio.ini`). |
| Wi‑Fi | internal | N/A | Uses ESP32-C3 on‑chip Wi‑Fi (no external pins required). |
| Power (LEDs) | 5V or external supply | LED VCC | Provide adequate current for the LED strip; use a large decoupling capacitor near the strip. |

## Reserved / avoid

- **GPIO2, GPIO8, GPIO9** — strapping pins; GPIO9 is also the onboard BOOT button and GPIO8 usually drives the onboard WS2812 status LED. Avoid wiring external signals here.
- **GPIO18/GPIO19** — used internally for native USB D-/D+ on some Super Mini boards; avoid reusing.
- **GPIO21** — default UART0 TX (unused when native USB CDC is enabled).

## Buzzer and switch

- The buzzer alert plays three rising notes. A passive piezo buzzer supports different pitches; an active buzzer may sound at a fixed pitch for each beep.
- The switch uses GPIO10 with `INPUT_PULLUP`; wire it between GPIO10 and GND. The firmware debounces presses and plays one alert per press.

## Notes

- Drive the WS2812B data line from a 3.3V-tolerant data pin; when powering the LEDs with 5V, a level shifter or resistor and proper signal conditioning is recommended.
- Keep a common ground between the ESP32-C3 and the LED power supply.
- Use a 1000 µF electrolytic capacitor across the LED power rails to prevent voltage spikes when the strip draws current.
- Confirm your OLED module voltage (many SSD1306/SH1106 modules accept 3.3V or 5V). If uncertain, power from 3.3V to be safe.
- ESP32-C3 is single-core RISC-V (no dual-core FreeRTOS pinning); if the code assumes a second core, review `main.cpp`/task setup accordingly.
