#include "wifiManager.h"

WiFiManager::WiFiManager(const char* ssid, const char* password) 
  : ssid(ssid), password(password), lastReconnectAttempt(0) {}

void WiFiManager::connect() {
  if(WiFi.status() == WL_CONNECTED) return;
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  
  int attempts = 0;
  while(WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nConnection failed!");
  }
}

bool WiFiManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::maintain() {
  if(!isConnected()) {
    unsigned long now = millis();
    if(now - lastReconnectAttempt > 5000) { // Reconnect every 5s
      lastReconnectAttempt = now;
      WiFi.disconnect();
      connect();
    }
  }
}