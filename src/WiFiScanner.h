#pragma once
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>

void ScanWifiCallback(AsyncWebServerRequest *request);