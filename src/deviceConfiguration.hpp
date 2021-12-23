#pragma once
#include <LittleFS.h>
#include <Arduino.h>
#include <ArduinoJson.h>

#define DEVICECONFIG "/conf/deviceConfiguration.json"

class deviceConfiguration
{
private:
    /* data */
    void createDefaultConfiguration();
public:
    void reloadConfig();
    const char* getFilename();
    deviceConfiguration(/* args */);
    ~deviceConfiguration();
};


