# AiluClock (smartClock)

An ESP32-C3 "smart clock" alert device plus its companion Android app, **AiluClock**.

The clock has a 5-LED WS2812B strip, an SSD1306 OLED, and a passive buzzer. The
Android app talks to it over a **USB OTG cable** (serial, 115200 baud) and can fire
alerts on a schedule: at the alarm time the phone sends a single `TRIGGER` command
that sets the LED color, shows a symbol on the OLED, and optionally sounds the buzzer.

## Repository layout

| Path | What it is |
|---|---|
| [src/](src) | ESP32-C3 firmware (Arduino framework, PlatformIO) |
| [android_app/](android_app) | AiluClock Android app (Kotlin + Jetpack Compose) |
| [docs/pin_connections.md](docs/pin_connections.md) | Wiring: GPIO map, power notes |
| [docs/serial_commands.md](docs/serial_commands.md) | The serial command protocol |
| [platformio.ini](platformio.ini) | PlatformIO env `esp32c3_supermini` |
| [include/](include) | Secrets templates (Wi-Fi creds if ever needed) |

## Hardware

- **ESP32-C3 Super Mini** (native USB CDC — the serial port *is* the USB-C port)
- **5× WS2812B** LED strip on GPIO20
- **SSD1306 OLED** 128×64 on I2C (SDA GPIO5, SCL GPIO6, addr `0x3C`)
- **Passive buzzer** on GPIO7

Full pin table and power/wiring guidance: [docs/pin_connections.md](docs/pin_connections.md).

> ⚠️ **Known hardware note:** with 3.3V data into a 5V-powered strip, the first LED's
> data input is marginal. The firmware works around it by re-sending the frame every
> 500 ms. The proper fixes are a level shifter (74AHCT125), a ~470 Ω series resistor
> on DIN plus a 1000 µF cap on the rails, or replacing pixel #1 if it was damaged by
> hot-plugging data before power.

## Firmware

### Build & flash

Requires [PlatformIO](https://platformio.org/) (CLI or the VS Code extension):

```powershell
pio run --environment esp32c3_supermini            # build
pio run --target upload --environment esp32c3_supermini  # flash over USB
pio device monitor -b 115200                       # serial monitor
```

### Serial protocol

One newline-terminated command per line at **115200 baud**:

```
TRIGGER R G B Emoji Sound
```

- `R G B` — LED array color, 0–255 per channel
- `Emoji` — OLED symbol; the firmware font (`u8g2_font_unifont_t_symbols`) only has
  simple symbols: **♥ ★ ☀ ☁ ☂ ♪ ⚡ ✓ ✗**
- `Sound` — `1` plays the buzzer alert, `0` stops it

Example: `TRIGGER 255 0 0 ♥ 1` → red LEDs, heart on OLED, buzzer alert.
Replies: `ACK TRIGGER` / `ERR BAD_ARGS TRIGGER` / `ERR UNKNOWN_CMD ...`.
Separators may be spaces or commas, so `TRIGGER(255,0,0,♥,1)` also works.

Details: [docs/serial_commands.md](docs/serial_commands.md).

### Boot behavior

On power-up: green LEDs, ♥ on the OLED, one startup beep. The clock is **stateless** —
there is intentionally no persistence on the device; the phone app provides it via
Color Profiles and alarms.

## Android app (AiluClock)

See [android_app/README.md](android_app/README.md) for full build/install instructions.
Highlights:

- **Alarms** — one-time (date+time) or daily; each alarm carries its own
  color/symbol/buzzer settings and fires the `TRIGGER` command over OTG at the set
  time. Exact alarms work in doze mode; alarms re-arm after reboot; if the clock is
  unplugged, a phone notification is the fallback alert.
- **Color Profiles** — named saved looks (color + symbol + buzzer); tap to apply
  instantly. This is "app-side persistence": the clock stays stateless, the phone
  reapplies your look on demand.
- **Connect / Test** — USB permission + a test button that fires
  `TRIGGER 255 0 0 ♥ 1` and shows the clock's reply.

Requirements: Android 8.0+, USB OTG support, an OTG cable. The app auto-launches when
the clock is plugged in (ESP32-C3 USB VID `0x303A`).

## Quick start

1. Wire the hardware per [docs/pin_connections.md](docs/pin_connections.md).
2. Flash the firmware (`pio run -t upload`).
3. Build the app in Android Studio (`android_app/`), install the APK on your phone.
4. Connect the clock to the phone with an OTG cable → grant USB permission → **Test**.
5. Create a Color Profile for one-tap looks, or an alarm for scheduled alerts.
