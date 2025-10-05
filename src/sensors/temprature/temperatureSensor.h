// sensors/temperatureSensor.h
#pragma once
#include "../mqttHandler/mqttHandler.h"
#include <OneWire.h>
#include <DallasTemperature.h>

class TemperatureSensor {
public:
    TemperatureSensor(MqttHandler& mqttHandler, uint8_t pin, const char* topic = "sensors/temperature")
        : mqtt(mqttHandler), oneWire(pin), sensors(&oneWire), publishTopic(topic) {}
    
    void begin() {
        sensors.begin();
        lastPublishTime = millis();
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