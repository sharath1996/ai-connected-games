"""Shared MQTT connection settings.

Reads from Streamlit's st.secrets when deployed (Streamlit Community Cloud),
falling back to pycode/.env for local development.
"""
import os
from pathlib import Path

try:
    import streamlit as st
    _secrets = st.secrets
except Exception:
    _secrets = {}

if not _secrets:
    from dotenv import load_dotenv
    load_dotenv(Path(__file__).parent / ".env")


def _get(key: str) -> str:
    return _secrets[key] if key in _secrets else os.environ[key]


MQTT_HOST = _get("MQTT_HOST")
MQTT_PORT = int(_get("MQTT_PORT"))
MQTT_USER = _get("MQTT_USER")
MQTT_PASSWORD = _get("MQTT_PASSWORD")
