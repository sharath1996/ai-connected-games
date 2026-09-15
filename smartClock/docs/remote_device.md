it should accept the array of 128*64 pixels 
## Device I/O Reference (Minimal)
Purpose: concise reference for the two active device services exposed over MQTT.

| Service | MQTT Topic | Direction | Simple payload example | Notes |
|---|---:|---|---|---|
| OLED display | `device/output/oled` | command -> device | `{ "data": "<base64>" }` | 128×64 monochrome, packed bytes (1024 bytes). Base64 recommended for simplicity. |
| LED strip | `device/output/ledstrip` | command -> device | `{ "data": [[R,G,B],[R,G,B],...] }` | Array index 0 = first LED. R/G/B = 0–255. |

Acknowledgement (simple)

Topic: `device/ack`

Simple ACK payload:

`{ "requestId":"...", "status":"ok" }` or `{ "requestId":"...", "status":"error", "error":"message" }`

Notes
- For now only the two services above are supported.
- Use QoS 1 for important commands. Keep payloads small; reject oversize data with an error ACK.