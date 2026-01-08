#include <Arduino.h>
#include "wiFiManager/wifiManager.h"
#include "mqttHandler/mqttHandler.h"
#include "sensors/ec/ecSensor.h"
#include "sensors/temprature/temperatureSensor.h"
#include "sensors/dht/dhtSensor.h"
#include "sensors/ph/phSensor.h"

// Configurations
const char* WIFI_SSID = "Qwerty 2.4G";
const char* WIFI_PASS = "Kshitij0509";
const char* MQTT_SERVER = "192.168.1.200";
const int MQTT_PORT = 1883;

// Manager instances
WiFiManager wifi(WIFI_SSID, WIFI_PASS);
MqttHandler mqtt(wifi, MQTT_SERVER, MQTT_PORT);

// Sensor instances
ECSensor ecSensor(mqtt, 32, "sensors/ec");
TemperatureSensor tempSensor(mqtt, 2, "sensors/temperature");
DHTSensor dhtSensor(mqtt, 4);  // DHT22 on GPIO 4
PHSensor phSensor(mqtt, 35, "sensors/ph");  // pH sensor on GPIO 35

// MQTT Callback function
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Handle incoming MQTT messages here
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(9600);
  
  // Initialize WiFi
  wifi.connect();
  
  // Set up MQTT
  mqtt.setCallback(mqttCallback);
  mqtt.connect();
  
  // Subscribe to topics if needed
  // mqtt.subscribe("sensors/command");
  
  // Initialize sensors
  ecSensor.begin();
  tempSensor.begin();
  dhtSensor.begin();
  phSensor.begin();
}

void loop() {
  // Maintain WiFi and MQTT connections
  wifi.maintain();
  mqtt.loop();
  
  // Update temperature sensor first
  tempSensor.update();
  
  // Update DHT sensor
  dhtSensor.update();
  
  // Get current temperature and update EC sensor for accurate compensation ec calibration
  float currentTemp = tempSensor.getLastTemperature();
  if (currentTemp > -1.0) {  // Valid temperature reading
    ecSensor.setTemperature(currentTemp);  // This updates line 34 temperature variable
  }
  
  // Update EC sensor (will now use actual temperature in calculation)
  ecSensor.update();
  
  // Update pH sensor
  phSensor.update();
  
  // Heartbeat
  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat > 30000) { // Every 30 seconds
    lastHeartbeat = millis();
    if (mqtt.isConnected()) {
      mqtt.publish("status/heartbeat", "alive");
    }
  }
  
  // Small delay to prevent watchdog issues
  delay(10);
}