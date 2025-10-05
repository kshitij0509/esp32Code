#pragma once
#include <PubSubClient.h>
#include <WiFiClient.h>

class WiFiManager; // Forward declaration

class MqttHandler {
public:
  MqttHandler(WiFiManager& wifiManager, const char* server, int port);
  
  // Connection management
  bool connect();
  void disconnect();
  bool isConnected() const;
  
  // MQTT operations
  bool publish(const char* topic, const char* payload);
  bool subscribe(const char* topic);
  void setCallback(MQTT_CALLBACK_SIGNATURE);
  
  // Main loop processing
  void loop();
  
private:
  WiFiManager& wifiManager;
  const char* server;
  int port;
  WiFiClient espClient;
  mutable PubSubClient client;
  
  // MQTT client settings
  static constexpr const char* MQTT_CLIENT_ID = "ESP32Client";
  static constexpr unsigned long RECONNECT_INTERVAL_MS = 5000;
  unsigned long lastReconnectAttempt = 0;
  
  bool reconnect();
};