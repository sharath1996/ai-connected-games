#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <U8g2lib.h>
#include <FastLED.h>
#include <ArduinoJson.h>
#include "secrets.h"
#include "netmgr.h"
#include "oled.h"
#include "ledarray.h"

const char* MQTT_TOPIC_OLED = "device/output/oled";
const char* MQTT_TOPIC_LED = "device/output/ledstrip";

// ============================================================
// MQTT callback routes to service handlers
// ============================================================
// MQTT callback routes to service handlers
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String t = String(topic);
  String msg;
  for (unsigned int i = 0; i < length; ++i) msg += (char)payload[i];
  Serial.print("MQTT msg on "); Serial.print(t); Serial.print(": "); Serial.println(msg);

  StaticJsonDocument<2048> doc;
  DeserializationError err = deserializeJson(doc, msg);
  if (err) { Serial.println("JSON parse failed"); publishAck(nullptr, "error", "json parse"); return; }

  if (t.equals(MQTT_TOPIC_OLED)) {
    oledHandleCommand(doc);
  } else if (t.equals(MQTT_TOPIC_LED)) {
    ledHandleCommand(doc);
  }
}

void setup() {
  Serial.begin(115200);
  oledInit();
  ledInit();
  wifiInit();
  setMqttCallback(mqttCallback);
  ensureMqttConnected();
  mqttSubscribe(MQTT_TOPIC_OLED);
  mqttSubscribe(MQTT_TOPIC_LED);
}

void loop() {
  ensureMqttConnected();
  mqttLoop();
  delay(10);
}
