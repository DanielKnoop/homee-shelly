#include "deviceConfiguration.hpp"

deviceConfiguration::deviceConfiguration(/* args */)
{
}

deviceConfiguration::~deviceConfiguration()
{
}

const char* deviceConfiguration::getFilename()
{
    return DEVICECONFIG;
}

void deviceConfiguration::createDefaultConfiguration()
{
    DynamicJsonDocument doc(1024);
    #if defined(SCHELLY_INPUTS)
    JsonArray inputs = doc.createNestedArray("inputType");
    for(int i = 0; i < SCHELLY_INPUTS; i++)
    {
        JsonObject input = inputs.createNestedObject();
        input["id"] = i + 1;
        input["type"] = 0;
        input["enabled"] = true;
        input["doubleClickEnabled"] = true;
        input["longClickEnabled"] = true;
        input["gpio"] = SCHELLY_INPUT_GPIO[i];
    }
    #endif

    File deviceConfig = LittleFS.open(DEVICECONFIG, "w");
    serializeJson(doc, deviceConfig);
    deviceConfig.close();
}