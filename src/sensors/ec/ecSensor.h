// sensors/ECSensor.h
#pragma once
#include "../../mqttHandler/mqttHandler.h"

class ECSensor {
public:
    ECSensor(MqttHandler& mqttHandler, uint8_t pin, const char* topic = "sensors/ec")
        : mqtt(mqttHandler), pin(pin), publishTopic(topic) {
        pinMode(pin, INPUT);
    }
    
    void begin() {
        lastPublishTime = millis();
        randomSeed(analogRead(39));  // Initialize random seed
    }
    
    void update() {
        if (millis() - lastPublishTime >= PUBLISH_INTERVAL && mqtt.isConnected()) {
            float ecValue = readEC();
            publishData(ecValue);
            lastPublishTime = millis();
        }
    }

    void setTemperature(float temp) {
        temperature = temp;
    }

private:
    static const unsigned long PUBLISH_INTERVAL = 5000;  // 5 seconds
    MqttHandler& mqtt;
    uint8_t pin;
    const char* publishTopic;
    float temperature = 25.0;  // Default temperature
    unsigned long lastPublishTime = 0;
    
    // MongoDB crop ID
    const char* cropId = "68c93e6e8fbcffb93a2393d5";

    float readEC() {
        float conversionFactor = 0.5;
        int analogSum = 0;
        
        for (int i = 0; i < 10; i++) {
            analogSum += analogRead(pin);
            delay(10);
        }
        // ec calibration through temperature
        float voltage = (analogSum / 10.0) * (3.3 / 4095.0);
        float tdsValue = (133.42 * pow(voltage, 3) - 255.86 * pow(voltage, 2) + 857.39 * voltage) * 0.5;
        tdsValue = tdsValue / (1.0 + 0.02 * (temperature - 25.0));
        
        return tdsValue / conversionFactor;
    }

    void publishData(float ecValue) {
        char payload[80];
        snprintf(payload, sizeof(payload), "{\"crop_id\":\"%s\",\"ec_value\":%.2f}", 
                cropId, ecValue);
        mqtt.publish(publishTopic, payload);
        Serial.print("EC Value: ");
        Serial.println(ecValue);
    }
};