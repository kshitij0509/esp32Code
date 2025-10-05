#pragma once
#include "../mqttHandler/mqttHandler.h"

class SensorBase {
public:
    SensorBase(MqttHandler& mqttHandler, const char* topic, unsigned long interval)
        : mqtt(mqttHandler), publishTopic(topic), publishInterval(interval) {}
    
    virtual void begin() = 0;
    virtual void update() = 0;
    
protected:
    MqttHandler& mqtt;
    const char* publishTopic;
    unsigned long publishInterval;
    unsigned long lastPublishTime = 0;

    bool shouldPublish() {
        return millis() - lastPublishTime >= publishInterval;
    }

    void resetPublishTimer() {
        lastPublishTime = millis();
    }
};