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
EMOJI_FONT_CANDIDATES = [
    "seguiemj.ttf",
    "/usr/share/fonts/truetype/noto/NotoColorEmoji.ttf",
    "NotoColorEmoji.ttf",
]
FONT_SIZE = 56
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
            try:
                return ImageFont.truetype(path, FONT_SIZE)
            except OSError as exc:
                last_error = exc
        raise DisplayEmojiError(f"No emoji font available on this system: {last_error}")

    @staticmethod
    def _render_to_pixels(emoji: str, threshold: int = THRESHOLD) -> str:
        if not emoji:
            raise DisplayEmojiError("Emoji text is empty")
        font = DisplayEmoji._load_font()
        canvas = Image.new("RGBA", (WIDTH, HEIGHT), (255, 255, 255, 255))
        draw = ImageDraw.Draw(canvas)
        try:
            bbox = draw.textbbox((0, 0), emoji, font=font, embedded_color=True)
            text_w, text_h = bbox[2] - bbox[0], bbox[3] - bbox[1]
            pos = ((WIDTH - text_w) // 2 - bbox[0], (HEIGHT - text_h) // 2 - bbox[1])
            draw.text(pos, emoji, font=font, embedded_color=True)
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
