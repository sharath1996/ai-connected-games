"""Publish a test color pattern to device/output/ledstrip over MQTT.

Pattern: a rainbow-ish gradient across the 10 pixels, so it's easy to
visually verify whether the LED strip actually updates from an MQTT message.
"""
import json
import ssl

import paho.mqtt.client as mqtt

from mqtt_config import MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASSWORD

TOPIC = "device/output/ledstrip"

NUM_PIXELS = 10

COLORS = [
    (255, 0, 0), (255, 85, 0), (255, 170, 0), (255, 255, 0), (170, 255, 0),
    (0, 255, 0), (0, 255, 170), (0, 170, 255), (0, 0, 255), (170, 0, 255),
]


def build_pattern() -> list:
    return [list(COLORS[i % len(COLORS)]) for i in range(NUM_PIXELS)]


def main() -> None:
    payload = json.dumps({"data": build_pattern(), "brightness": 128})
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
