|Description|Serial Command | Arguments | full serial command|
|------------|--------------|-----------|--------------------|
|Trigger alert: set LED array color, show emoji on OLED, optionally sound the buzzer| TRIGGER | R, G, B, Emoji, Sound (1/0) | TRIGGER 255 0 0 ♥ 1 |
|Persist power-on LED defaults (color + optional brightness)| PERSIST_LED | R, G, B [, Brightness 1-255] | PERSIST_LED 0 128 255 160 |
|Persist power-on OLED symbol| PERSIST_EMOJI | Emoji | PERSIST_EMOJI ★ |

- **R, G, B** — color for the whole LED array, 0-255 per channel.
- **Emoji** — character/emoji to show on the OLED display.
- **Sound** — `1` plays the buzzer alert, `0` stops the buzzer.
- **Brightness** — optional in `PERSIST_LED`; if omitted, the previously stored brightness is kept.

`PERSIST_*` commands only **save** the defaults to flash (ESP32 Preferences/NVS, the
equivalent of EEPROM — wear-leveled, survives power loss). They do not change what is
currently displayed; use `TRIGGER` for that. The stored color/brightness/emoji are
applied automatically at every boot. Because flash has a limited number of write
cycles (~100k), use PERSIST to set defaults occasionally — not in a fast loop.
`TRIGGER` also remembers the last emoji shown, so a reboot restores the most recent one.

Arguments may be separated by spaces or commas, so `TRIGGER(255,0,0,♥,1)` is also accepted.

And if the switch is pressed, it should be defaulting to off or normal position

## Transport

Commands are accepted, one per line (newline-terminated), over **USB Serial** at 115200 baud.
