#pragma once
#include <Arduino.h>
#include "../../mqttHandler/mqttHandler.h"
#include <DHT.h>

class DHTSensor {
public:
    DHTSensor(MqttHandler& mqttHandler, uint8_t pin, uint8_t dhtType = DHT22, 
              const char* tempTopic = "sensors/dht/temperature", 
              const char* humidityTopic = "sensors/dht/humidity")
        : mqtt(mqttHandler), dht(pin, dhtType), tempPublishTopic(tempTopic), 
          humidityPublishTopic(humidityTopic) {}
    
    void begin() {
        Serial.println("=== DHT Sensor Begin ===");
        dht.begin();
        lastPublishTime = millis();
        Serial.println("=== DHT Sensor Init Complete ===");
    }
    
    void update() {
        if (millis() - lastPublishTime >= PUBLISH_INTERVAL && mqtt.isConnected()) {
            readSensorData();
            publishData();
            lastPublishTime = millis();
        }
    }

    float getLastTemperature() const { return lastTemperature; }
    float getLastHumidity() const { return lastHumidity; }

private:
    static const unsigned long PUBLISH_INTERVAL = 10000;  // 10 seconds
    MqttHandler& mqtt;
    DHT dht;
    const char* tempPublishTopic;
    const char* humidityPublishTopic;
    float lastTemperature = 0.0;
    float lastHumidity = 0.0;
    unsigned long lastPublishTime = 0;

    void readSensorData() {
        float temp = dht.readTemperature();
        float humidity = dht.readHumidity();
        
        if (isnan(temp) || isnan(humidity)) {
            Serial.println("Error: Failed to read from DHT sensor!");
            return;
        }
        
        lastTemperature = temp;
        lastHumidity = humidity;
    }

    void publishData() {
        if (!isnan(lastTemperature) && !isnan(lastHumidity)) {
            char tempPayload[8];
            char humidityPayload[8];
            
            dtostrf(lastTemperature, 5, 2, tempPayload);
            dtostrf(lastHumidity, 5, 2, humidityPayload);
            
            mqtt.publish(tempPublishTopic, tempPayload);
            mqtt.publish(humidityPublishTopic, humidityPayload);
            
            Serial.print("Published DHT Temperature: ");
            Serial.print(tempPayload);
            Serial.print(" °C, Humidity: ");
            Serial.print(humidityPayload);
            Serial.println(" %");
        }
    }
};