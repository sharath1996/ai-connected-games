"""Publish a 128x64 raw pixel-string test pattern to device/output/oled over MQTT.

Pattern: a full border box, so it's easy to visually verify whether the OLED
actually updates from an MQTT message.
"""
import json
import ssl

import paho.mqtt.client as mqtt

from mqtt_config import MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASSWORD

TOPIC = "device/output/oled"

WIDTH, HEIGHT = 128, 64


def build_pattern() -> str:
    pixels = []
    for y in range(HEIGHT):
        for x in range(WIDTH):
            on_border = x == 0 or x == WIDTH - 1 or y == 0 or y == HEIGHT - 1
            pixels.append('1' if on_border else '0')
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
