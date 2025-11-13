// sensors/temperatureSensor.h
#pragma once
#include <Arduino.h>
#include "../../mqttHandler/mqttHandler.h"
#include <OneWire.h>
#include <DallasTemperature.h>

class TemperatureSensor {
public:
    TemperatureSensor(MqttHandler& mqttHandler, uint8_t pin, const char* topic = "sensors/temperature")
        : mqtt(mqttHandler), oneWire(pin), sensors(&oneWire), publishTopic(topic) {}
    
    void begin() {
        Serial.println("=== Temperature Sensor Begin ===");
        pinMode(2, INPUT_PULLUP);  // Enable internal pull-up
        sensors.begin();
        
        // Debug: Check if sensor is detected
        int deviceCount = sensors.getDeviceCount();
        Serial.print("DS18B20 devices found: ");
        Serial.println(deviceCount);
        
        if (deviceCount == 0) {
            Serial.println("No DS18B20 found! ADD 4.7k RESISTOR between GPIO2 and 3.3V!");
        } else {
            Serial.println("DS18B20 detected successfully!");
        }
        
        lastPublishTime = millis();
        Serial.println("=== Temperature Sensor Init Complete ===");
    }
    
    void update() {
        if (millis() - lastPublishTime >= PUBLISH_INTERVAL && mqtt.isConnected()) {
            float tempC = readTemperature();
            publishData(tempC);
            lastPublishTime = millis();
        }
    }

    float getLastTemperature() const {
        return lastTemperature;
    }

private:
    static const unsigned long PUBLISH_INTERVAL = 10000;  // 10 seconds
    MqttHandler& mqtt;
    OneWire oneWire;
    DallasTemperature sensors;
    const char* publishTopic;
    float lastTemperature = 0.0;
    unsigned long lastPublishTime = 0;

    float readTemperature() {
        sensors.requestTemperatures();
        float tempC = sensors.getTempCByIndex(0);
        
        if (tempC == DEVICE_DISCONNECTED_C) {
            Serial.println("Error: DS18B20 sensor not connected!");
            return -1.0;  // Return error value
        }
        
        lastTemperature = tempC;
        return tempC;
    }

    void publishData(float tempC) {
        char payload[8];
        dtostrf(tempC, 5, 2, payload);
        mqtt.publish(publishTopic, payload);
        Serial.print("Published Temperature: ");
        Serial.print(payload);
        Serial.println(" °C");
    }
};