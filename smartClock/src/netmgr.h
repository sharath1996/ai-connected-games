#pragma once
#include <Arduino.h>

// Initialize network and MQTT (connect to network)
void wifiInit();

// Ensure MQTT connection is active
void ensureMqttConnected();

// Call regularly to service MQTT
void mqttLoop();

// Publish helper
bool mqttPublish(const char* topic, const char* payload);

// Subscribe helper
void mqttSubscribe(const char* topic);

// Set the MQTT message callback (PubSubClient signature)
void setMqttCallback(void (*callback)(char*, uint8_t*, unsigned int));

// Publish an ACK object to device/ack
void publishAck(const char* requestId, const char* status, const char* message=nullptr);
