#pragma once
#include <Arduino.h>
#include "../../mqttHandler/mqttHandler.h"

class PHSensor {
public:
    PHSensor(MqttHandler& mqttHandler, uint8_t analogPin, 
             const char* phTopic = "sensors/ph/value")
        : mqtt(mqttHandler), pin(analogPin), publishTopic(phTopic) {}
    
    void begin() {
        Serial.println("=== pH Sensor Begin ===");
        pinMode(pin, INPUT);
        lastPublishTime = millis();
        Serial.println("=== pH Sensor Init Complete ===");
    }
    
    void update() {
        if (millis() - lastPublishTime >= PUBLISH_INTERVAL && mqtt.isConnected()) {
            readSensorData();
            publishData();
            lastPublishTime = millis();
        }
    }

    float getLastPH() const { return lastPH; }

private:
    static const unsigned long PUBLISH_INTERVAL = 10000;  // 10 seconds
    static const int SAMPLES = 50;  // More samples for stability
    
    MqttHandler& mqtt;
    uint8_t pin;
    const char* publishTopic;
    float lastPH = 7.0;
    unsigned long lastPublishTime = 0;
    float voltageBuffer[5] = {1.65, 1.65, 1.65, 1.65, 1.65};  // Moving average buffer
    int bufferIndex = 0;
    
    // Calibration values (adjust based on your sensor)
    const float VOLTAGE_REF = 3.3;  // ESP32 reference voltage
    const float PH_NEUTRAL = 7.0;
    const float VOLTAGE_NEUTRAL = 1.65;  // Voltage at pH 7
    const float PH_SLOPE = -3.5;  // pH per volt

    void readSensorData() {
        long sum = 0;
        for (int i = 0; i < SAMPLES; i++) {
            sum += analogRead(pin);
            delay(5);
        }
        
        float avgReading = sum / (float)SAMPLES;
        float voltage = (avgReading / 4095.0) * VOLTAGE_REF;
        
        // Add to moving average buffer
        voltageBuffer[bufferIndex] = voltage;
        bufferIndex = (bufferIndex + 1) % 5;
        
        // Calculate moving average
        float avgVoltage = 0;
        for (int i = 0; i < 5; i++) {
            avgVoltage += voltageBuffer[i];
        }
        avgVoltage /= 5.0;
        
        // Convert voltage to pH using linear calibration
        lastPH = PH_NEUTRAL + ((avgVoltage - VOLTAGE_NEUTRAL) * PH_SLOPE);
        
        // Clamp pH to realistic range
        if (lastPH < 0) lastPH = 0;
        if (lastPH > 14) lastPH = 14;
        
        Serial.print("pH Raw Voltage: ");
        Serial.print(avgVoltage, 3);
        Serial.print("V, pH: ");
        Serial.println(lastPH, 2);
    }

    void publishData() {
        // Check if sensor is connected (similar to EC sensor)
        float rawReading = analogRead(pin);
        char payload[120];
        
        if (rawReading <= 10 || rawReading >= 4085) {
            // Sensor disconnected
            snprintf(payload, sizeof(payload), 
                    "{\"crop_id\":\"68c93e6e8fbcffb93a2393d5\",\"ph\":null,\"status\":\"disconnected\"}");
        } else {
            // Sensor connected
            snprintf(payload, sizeof(payload), 
                    "{\"crop_id\":\"68c93e6e8fbcffb93a2393d5\",\"ph\":%.2f,\"status\":\"connected\"}", 
                    lastPH);
        }
        
        mqtt.publish(publishTopic, payload);
        
        Serial.print("Published pH: ");
        Serial.println(lastPH, 2);
    }
};