#pragma once
#include <WiFi.h>

class WiFiManager {
public:
  WiFiManager(const char* ssid, const char* password);
  void connect();
  bool isConnected();
  void maintain(); // Non-blocking connection maintenance
  
private:
  const char* ssid;
  const char* password;
  unsigned long lastReconnectAttempt;
};