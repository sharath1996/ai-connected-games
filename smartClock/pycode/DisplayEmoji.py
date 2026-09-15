"""Render any emoji to the OLED via MQTT.

Usage:
    from DisplayEmoji import DisplayEmoji
    DisplayEmoji.display("\U0001F600")  # grinning face

Or from the command line:
    python DisplayEmoji.py "\U0001F600"
"""
import json
import ssl
import sys

import paho.mqtt.client as mqtt
from PIL import Image, ImageDraw, ImageFont

from mqtt_config import MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASSWORD

TOPIC = "device/output/oled"

WIDTH, HEIGHT = 128, 64
# Windows built-in color emoji font, with fallbacks for Linux/cloud environments
# (installed via packages.txt: fonts-noto-color-emoji, fonts-noto-emoji)
EMOJI_FONT_CANDIDATES = [
    "seguiemj.ttf",
    "/usr/share/fonts/truetype/noto/NotoColorEmoji.ttf",
    "/usr/share/fonts/truetype/noto-emoji/NotoColorEmoji.ttf",
    "/usr/share/fonts/truetype/noto/NotoEmoji-Regular.ttf",
    "/usr/share/fonts/truetype/noto-emoji/NotoEmoji-Regular.ttf",
    "NotoColorEmoji.ttf",
]
FONT_SIZE = 56
# Color bitmap fonts (e.g. NotoColorEmoji) only support fixed strike sizes; try a few
FONT_SIZE_CANDIDATES = [FONT_SIZE, 128, 136, 109, 72, 64, 32]
THRESHOLD = 128  # luminance below this is treated as "on" (dark glyph on light bg)
MQTT_TIMEOUT = 10  # seconds


class DisplayEmojiError(Exception):
    """Raised when an emoji fails to render or publish, with a user-friendly message."""


class DisplayEmoji:
    """Renders an emoji to a monochrome bitmap and publishes it to the OLED over MQTT."""

    @staticmethod
    def _load_font() -> ImageFont.FreeTypeFont:
        last_error = None
        for path in EMOJI_FONT_CANDIDATES:
            for size in FONT_SIZE_CANDIDATES:
                try:
                    return ImageFont.truetype(path, size)
                except OSError as exc:
                    last_error = exc
        raise DisplayEmojiError(f"No emoji font available on this system: {last_error}")

    @staticmethod
    def _render_to_pixels(emoji: str, threshold: int = THRESHOLD) -> str:
        if not emoji:
            raise DisplayEmojiError("Emoji text is empty")
        font = DisplayEmoji._load_font()
        try:
            # render on a generous scratch canvas first, since the loaded font's
            # native strike size may not match our target dimensions
            scratch = Image.new("RGBA", (256, 256), (255, 255, 255, 255))
            draw = ImageDraw.Draw(scratch)
            bbox = draw.textbbox((0, 0), emoji, font=font, embedded_color=True)
            draw.text((0, 0), emoji, font=font, embedded_color=True)
            glyph = scratch.crop(bbox)
            glyph.thumbnail((WIDTH, HEIGHT))

            canvas = Image.new("RGBA", (WIDTH, HEIGHT), (255, 255, 255, 255))
            pos = ((WIDTH - glyph.width) // 2, (HEIGHT - glyph.height) // 2)
            canvas.paste(glyph, pos)
        except Exception as exc:
            raise DisplayEmojiError(f"Failed to render emoji: {exc}") from exc

        gray = canvas.convert("L")
        pixels = []
        for y in range(HEIGHT):
            for x in range(WIDTH):
                pixels.append('1' if gray.getpixel((x, y)) < threshold else '0')
        return ''.join(pixels)

    @staticmethod
    def display(emoji: str, threshold: int = THRESHOLD) -> None:
        payload = json.dumps({"data": DisplayEmoji._render_to_pixels(emoji, threshold)})
        client = mqtt.Client(protocol=mqtt.MQTTv311)
        client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
        client.tls_set(cert_reqs=ssl.CERT_NONE)
        client.tls_insecure_set(True)
        try:
            client.connect(MQTT_HOST, MQTT_PORT, keepalive=30)
            client.loop_start()
            info = client.publish(TOPIC, payload, qos=1)
            info.wait_for_publish(timeout=MQTT_TIMEOUT)
            if not info.is_published():
                raise DisplayEmojiError("Publish did not complete in time")
            print(f"Published {len(payload)} bytes to {TOPIC}")
        except DisplayEmojiError:
            raise
        except Exception as exc:
            raise DisplayEmojiError(f"MQTT connection/publish failed: {exc}") from exc
        finally:
            client.loop_stop()
            client.disconnect()


if __name__ == "__main__":
    emoji_arg = sys.argv[1] if len(sys.argv) > 1 else "\U0001F600"
    DisplayEmoji.display(emoji_arg)
