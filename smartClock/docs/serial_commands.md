|Description|Serial Command | Arguments | full serial command|
|------------|--------------|-----------|--------------------|
|Trigger alert: set LED array color, show emoji on OLED, optionally sound the buzzer| TRIGGER | R, G, B, Emoji, Sound (1/0) | TRIGGER 255 0 0 ♥ 1 |

- **R, G, B** — color for the whole LED array, 0-255 per channel.
- **Emoji** — character/emoji to show on the OLED display.
- **Sound** — `1` plays the buzzer alert, `0` stops the buzzer.

Arguments may be separated by spaces or commas, so `TRIGGER(255,0,0,♥,1)` is also accepted.

And if the switch is pressed, it should be defaulting to off or normal position

## Transport

Commands are accepted, one per line (newline-terminated), over **USB Serial** at 115200 baud.
