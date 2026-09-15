"""Streamlit control panel: send emojis to the OLED and colors to the LED strip.

Run with:
    .venv\\Scripts\\streamlit.exe run pycode/streamlit_app.py
"""
import json
import ssl

import emoji
import streamlit as st
import paho.mqtt.client as mqtt

from mqtt_config import MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASSWORD
from DisplayEmoji import DisplayEmoji

OLED_TOPIC = "device/output/oled"
LED_TOPIC = "device/output/ledstrip"
NUM_PIXELS = 10


@st.cache_data
def load_emoji_choices() -> list:
    """Full Unicode emoji list (char, name) from the `emoji` package, sorted by name."""
    choices = [
        (char, data["en"].strip(":").replace("_", " "))
        for char, data in emoji.EMOJI_DATA.items()
        if data.get("status") == emoji.STATUS["fully_qualified"]
    ]
    return sorted(choices, key=lambda pair: pair[1])


def publish(topic: str, payload: dict) -> None:
    client = mqtt.Client(protocol=mqtt.MQTTv311)
    client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    client.tls_set(cert_reqs=ssl.CERT_NONE)
    client.tls_insecure_set(True)
    client.connect(MQTT_HOST, MQTT_PORT, keepalive=30)
    client.loop_start()
    info = client.publish(topic, json.dumps(payload), qos=1)
    info.wait_for_publish()
    client.loop_stop()
    client.disconnect()


def hex_to_rgb(hex_color: str) -> list:
    hex_color = hex_color.lstrip("#")
    return [int(hex_color[i:i + 2], 16) for i in (0, 2, 4)]


st.set_page_config(page_title="SmartClock Control", page_icon="⏰")
st.title("SmartClock Control Panel")

st.header("😀 OLED Emoji")
if "emoji" not in st.session_state:
    st.session_state.emoji = "😀"

emoji_choices = load_emoji_choices()
st.caption(f"Search the full emoji list ({len(emoji_choices)} available):")
selected = st.selectbox(
    "Pick an emoji",
    options=emoji_choices,
    format_func=lambda pair: f"{pair[0]}  {pair[1]}",
    label_visibility="collapsed",
)
if st.button("Use selected emoji"):
    st.session_state.emoji = selected[0]

emoji_value = st.text_input("Emoji", key="emoji", max_chars=8)
if st.button("Send emoji to OLED"):
    with st.spinner("Sending..."):
        DisplayEmoji.display(emoji_value)
    st.success(f"Sent {emoji_value} to {OLED_TOPIC}")

st.divider()

st.header("💡 LED Strip")
mode = st.radio("Mode", ["Single color (all pixels)", "Per-pixel"], horizontal=True)
brightness = st.slider("Brightness", 0, 255, 128)

if mode == "Single color (all pixels)":
    color = st.color_picker("Color", value="#FF0000")
    colors = [hex_to_rgb(color) for _ in range(NUM_PIXELS)]
else:
    cols = st.columns(NUM_PIXELS)
    colors = []
    for i, col in enumerate(cols):
        with col:
            c = st.color_picker(f"{i}", value="#FF0000", key=f"pixel_{i}")
            colors.append(hex_to_rgb(c))

if st.button("Send colors to LED strip"):
    with st.spinner("Sending..."):
        publish(LED_TOPIC, {"data": colors[::-1], "brightness": brightness})
    st.success(f"Sent colors to {LED_TOPIC}")

if st.button("Turn LEDs off"):
    with st.spinner("Sending..."):
        publish(LED_TOPIC, {"data": [[0, 0, 0]] * NUM_PIXELS, "brightness": 0})
    st.success("LEDs turned off")
