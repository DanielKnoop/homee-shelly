#include <WiFiScanner.h>

void ScanWifiCallback(AsyncWebServerRequest *request)
{
    int32_t n = WiFi.scanComplete();
    
    if(n == -2)
    {
        WiFi.scanNetworks(true);
        request->send(200, "application/json", "{\"wifiList\":[]}");
    }
    else if(n) 
    {
        AsyncJsonResponse *response = new AsyncJsonResponse();
        JsonVariant jsonResponse = response->getRoot(); 
        JsonArray wifi = jsonResponse.createNestedArray("wifiList");

        for(int32_t i = 0; i < n; i++) {
            JsonObject wifio = wifi.createNestedObject();
            wifio["rssi"] = WiFi.RSSI(i);
            wifio["ssid"] = WiFi.SSID(i);
            wifio["secure"] = WiFi.encryptionType(i);
        }

        WiFi.scanDelete();
        if(WiFi.scanComplete() == -2)
        {
            WiFi.scanNetworks(true);
        }

        response->setLength();
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    }
}