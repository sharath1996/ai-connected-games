#include "netmgr.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

// MQTT broker settings (keep local to this module)
static const char* MQTT_HOST = "1209eebea1444967be8397ca788a74b9.s1.eu.hivemq.cloud";
static const uint16_t MQTT_PORT = 8883;
static const char* MQTT_TOPIC_ACK = "device/ack";

static WiFiClientSecure tlsClient;
static PubSubClient mqttClient(tlsClient);
static char mqttClientId[32];
static void (*userCallback)(char*, uint8_t*, unsigned int) = nullptr;

void wifiInit() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println("\nWiFi Connected");
}

void setMqttCallback(void (*callback)(char*, uint8_t*, unsigned int)) {
  userCallback = callback;
  mqttClient.setCallback(callback);
  // default 256-byte buffer is too small for the 128*64 raw-pixel OLED payload (~8.2KB)
  mqttClient.setBufferSize(9000);
}

void ensureMqttConnected() {
  if (mqttClient.connected()) return;
  Serial.print("Connecting to MQTT...");
  String mac = WiFi.macAddress();
  mac.replace(":", ""); mac.toLowerCase();
  snprintf(mqttClientId, sizeof(mqttClientId), "smartclock-%s", mac.c_str());
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  tlsClient.setInsecure();
  if (mqttClient.connect(mqttClientId, MQTT_USER, MQTT_PASSWORD)) {
    Serial.println(" connected");
    if (userCallback) mqttClient.setCallback(userCallback);
    // announce online
    publishAck(nullptr, "online", nullptr);
  } else {
    Serial.print(" failed, rc="); Serial.println(mqttClient.state());
  }
}

void mqttLoop() {
  mqttClient.loop();
}

bool mqttPublish(const char* topic, const char* payload) {
  return mqttClient.publish(topic, payload);
}

void mqttSubscribe(const char* topic) {
  if (mqttClient.connected()) mqttClient.subscribe(topic);
}

void publishAck(const char* requestId, const char* status, const char* message) {
  StaticJsonDocument<128> doc;
  if (requestId) doc["requestId"] = requestId;
  doc["status"] = status;
  if (message) doc["error"] = message;
  char buf[128]; size_t n = serializeJson(doc, buf);
  mqttPublish(MQTT_TOPIC_ACK, buf);
}
