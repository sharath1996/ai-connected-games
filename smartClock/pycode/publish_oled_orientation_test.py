"""Diagnostic pattern for figuring out OLED orientation/offset issues.

Draws a big asymmetric "F" (so rotation/mirroring is visually obvious) plus a
small filled square in the top-left corner (marks true origin) and publishes
it as the same 128*64 raw pixel-string format used elsewhere.
"""
import json
import ssl

import paho.mqtt.client as mqtt
from PIL import Image, ImageDraw, ImageFont

from mqtt_config import MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASSWORD

TOPIC = "device/output/oled"
WIDTH, HEIGHT = 128, 64
THRESHOLD = 128


def build_pattern() -> str:
    img = Image.new("L", (WIDTH, HEIGHT), 255)
    draw = ImageDraw.Draw(img)
    # 8x8 marker square pinned to the true top-left corner
    draw.rectangle([0, 0, 7, 7], fill=0)
    # large asymmetric "F" centered, to reveal any rotation/mirroring
    font = ImageFont.truetype("arialbd.ttf", 48)
    draw.text((WIDTH // 2 - 8, HEIGHT // 2 - 28), "F", font=font, fill=0)

    pixels = []
    for y in range(HEIGHT):
        for x in range(WIDTH):
            pixels.append('1' if img.getpixel((x, y)) < THRESHOLD else '0')
    return ''.join(pixels)


def main() -> None:
    payload = json.dumps({"data": build_pattern()})
    client = mqtt.Client(protocol=mqtt.MQTTv311)
    client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    client.tls_set(cert_reqs=ssl.CERT_NONE)
    client.tls_insecure_set(True)
    client.connect(MQTT_HOST, MQTT_PORT, keepalive=30)
    client.loop_start()
    info = client.publish(TOPIC, payload, qos=1)
    info.wait_for_publish()
    print(f"Published {len(payload)} bytes to {TOPIC}")
    client.loop_stop()
    client.disconnect()


if __name__ == "__main__":
    main()
