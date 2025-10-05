#include "mqttHandler.h"
#include "../wiFiManager/wifiManager.h"

MqttHandler::MqttHandler(WiFiManager& wifiManager, const char* server, int port)
  : wifiManager(wifiManager), 
    server(server), 
    port(port), 
    client(espClient) {
  client.setServer(server, port);
}

bool MqttHandler::connect() {
  if (!wifiManager.isConnected()) {
    Serial.println("MQTT: WiFi not connected, can't connect to MQTT");
    return false;
  }
  
  Serial.print("MQTT: Connecting to ");
  Serial.print(server);
  Serial.print(":");
  Serial.print(port);
  Serial.println("...");
  
  if (client.connect(MQTT_CLIENT_ID)) {
    Serial.println("MQTT: Connected");
    return true;
  } else {
    Serial.print("MQTT: Connection failed, rc=");
    Serial.println(client.state());
    return false;
  }
}

void MqttHandler::disconnect() {
  if (client.connected()) {
    client.disconnect();
    Serial.println("MQTT: Disconnected");
  }
}

bool MqttHandler::isConnected() const {
  return client.connected();
}

bool MqttHandler::publish(const char* topic, const char* payload) {
  if (!isConnected()) {
    Serial.println("MQTT: Not connected, can't publish");
    return false;
  }
  
  bool result = client.publish(topic, payload);
  if (!result) {
    Serial.println("MQTT: Publish failed");
  }
  return result;
}

bool MqttHandler::subscribe(const char* topic) {
  if (!isConnected()) {
    Serial.println("MQTT: Not connected, can't subscribe");
    return false;
  }
  
  bool result = client.subscribe(topic);
  if (result) {
    Serial.print("MQTT: Subscribed to ");
    Serial.println(topic);
  } else {
    Serial.print("MQTT: Subscribe failed for ");
    Serial.println(topic);
  }
  return result;
}

void MqttHandler::setCallback(MQTT_CALLBACK_SIGNATURE) {
  client.setCallback(callback);
}

bool MqttHandler::reconnect() {
  if (wifiManager.isConnected()) {
    return connect();
  }
  return false;
}

void MqttHandler::loop() {
  if (!client.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt >= RECONNECT_INTERVAL_MS) {
      lastReconnectAttempt = now;
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    client.loop();
  }
}