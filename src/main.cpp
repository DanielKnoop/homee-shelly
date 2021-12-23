#define ADC_RESOLUTION 1024.0f
#define DEVICECONFIG "/conf/deviceConfiguration.json"

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <virtualHomee.h>
#include <TaskScheduler.h>
#include "avdweb_Switch.h"
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include "web/web.h"
#include "WiFiScanner.h"

#include "deviceConfiguration.hpp"

#include "ota.h"
extern "C" {
#include "tempSensor.h"
}

//Foreward Declarations
void initializeDevice();

const char* hostname = "shelly-i3-homee";
const char* ssid = "X";
const char* password = "X";

virtualHomee* vhih;
nodeAttributes* i1;
nodeAttributes* i1_long;
nodeAttributes* i1_double;
nodeAttributes* i2;
nodeAttributes* i2_long;
nodeAttributes* i2_double;
nodeAttributes* i3;
nodeAttributes* i3_long;
nodeAttributes* i3_double;
nodeAttributes* temperature;

Switch input1(14, INPUT, true);
Switch input2(12, INPUT, true);
Switch input3(13, INPUT, true);

void tempReadCallback();
Scheduler ts;
Task readTempTask(60 * TASK_SECOND, TASK_FOREVER, &tempReadCallback, &ts, true);
Task reset_i1(500 * TASK_MILLISECOND, TASK_FOREVER, []() {reset_i1.disable();i1->setCurrentValue(2);vhih->updateAttribute(i1);}, &ts, false);
Task reset_i1_double(500 * TASK_MILLISECOND, TASK_FOREVER, []() {reset_i1_double.disable();i1_double->setCurrentValue(2);vhih->updateAttribute(i1_double);}, &ts, false);
Task reset_i2(500 * TASK_MILLISECOND, TASK_FOREVER, []() {reset_i2.disable();i2->setCurrentValue(2);vhih->updateAttribute(i2);}, &ts, false);
Task reset_i2_double(500 * TASK_MILLISECOND, TASK_FOREVER, []() {reset_i2_double.disable();i2_double->setCurrentValue(2);vhih->updateAttribute(i2_double);}, &ts, false);
Task reset_i3(500 * TASK_MILLISECOND, TASK_FOREVER, []() {reset_i3.disable();i3->setCurrentValue(2);vhih->updateAttribute(i3);}, &ts, false);
Task reset_i3_double(500 * TASK_MILLISECOND, TASK_FOREVER, []() {reset_i3_double.disable();i3_double->setCurrentValue(2);vhih->updateAttribute(i3_double);}, &ts, false);

AsyncWebServer webserver(80);
AsyncEventSource events("/events");

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
  Serial.println("Handle Upload");
}
void onUploadDeviceConfig(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  if(!index && LittleFS.exists(DEVICECONFIG))
  {
    LittleFS.remove(DEVICECONFIG);    
  }
    
  File configFile = LittleFS.open(DEVICECONFIG, "a");
  configFile.write(data, len);
  configFile.close();

  if(index + len == total)
    initializeDevice();
}

void initializeDevice()
{
  File deviceConfigFile = LittleFS.open(DEVICECONFIG, "r");
  DynamicJsonDocument deviceConfigJson(1024);
  deserializeJson(deviceConfigJson, deviceConfigFile);
  deviceConfigFile.close();
  serializeJsonPretty(deviceConfigJson, Serial);
}

void setup() {
  Serial.begin(115200);
  /*
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(false);
  WiFi.softAP(hostname);
  */

  LittleFS.begin();

  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  initOTA(hostname);
  
  webserver.serveStatic("/", LittleFS, "/web/").setDefaultFile("index.html");

  webserver.on("/assets/deviceConfiguration.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/conf/deviceConfiguration.json", "application/json");
    request->send(response);
  });

  webserver.on("/assets/deviceConfiguration.json", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200);
  }, handleUpload, onUploadDeviceConfig);

  webserver.on("/scan", HTTP_GET, ScanWifiCallback);

/*
  webserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gz, index_html_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  webserver.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gz, index_html_gz_len);
      response->addHeader("Content-Encoding", "gzip");
    request->send(response);  
  });
  webserver.on("/main.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "application/json", main_js_gz, main_js_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);  
  });
  webserver.on("/polyfills.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "application/json", polyfills_js_gz, polyfills_js_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);  
  });
  webserver.on("/runtime.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "application/json", runtime_js_gz, runtime_js_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);  
  });
  webserver.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css", styles_css_gz, styles_css_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);  
  });  

  */

  webserver.begin();

  vhih = new virtualHomee(hostname);
  node* shellyi3Node = new node(10, 25, "Shelly i3"); // 25 = ThreeButtonRemote
  
  i1        = shellyi3Node->AddAttributes(new nodeAttributes(40)); //ButtonState
  i1_double = shellyi3Node->AddAttributes(new nodeAttributes(171)); //DoubleClick
  i1_long   = shellyi3Node->AddAttributes(new nodeAttributes(310)); //LongPress 

  i2        = shellyi3Node->AddAttributes(new nodeAttributes(40)); //ButtonState
  i2_double = shellyi3Node->AddAttributes(new nodeAttributes(171)); //DoubleClick
  i2_long   = shellyi3Node->AddAttributes(new nodeAttributes(310)); //LongPress

  i3        = shellyi3Node->AddAttributes(new nodeAttributes(40)); //ButtonState
  i3_double = shellyi3Node->AddAttributes(new nodeAttributes(171)); //DoubleClick
  i3_long   = shellyi3Node->AddAttributes(new nodeAttributes(310)); //LongPress 

  temperature = shellyi3Node->AddAttributes(new nodeAttributes(92)); //DeviceTemperature

  i1->setMaximumValue(2);
  i1->setCurrentValue(2);
  i1_double->setMaximumValue(2);
  i1_double->setCurrentValue(2);
  i1_long->setMaximumValue(1);

  i2->setMaximumValue(2);
  i2->setCurrentValue(2);
  i2_double->setMaximumValue(2);
  i2_double->setCurrentValue(2);
  i2_long->setMaximumValue(1);

  i3->setMaximumValue(2);
  i3->setCurrentValue(2);
  i3_double->setMaximumValue(2);
  i3_double->setCurrentValue(2);
  i3_long->setMaximumValue(1);

  temperature->setUnit("°C");
  temperature->setMinimumValue(0);
  temperature->setMaximumValue(80);

  vhih->addNode(shellyi3Node);
  vhih->start();
}

void tempReadCallback() 
{
  int raw = analogRead(0);
  float vin_ = 3.3f;
  float rd_ = 33000.0f;
  float v_out = raw / ADC_RESOLUTION;
  float rt = (v_out * rd_) / (vin_ - v_out);
  float t = Interpolate(rt);
  temperature->setCurrentValue(t);
  vhih->updateAttribute(temperature);
}

void processButton(Switch *inputSwitch, nodeAttributes *s, nodeAttributes *d, nodeAttributes *l, Task *taskSingle, Task *taskDouble)
{
  inputSwitch->poll();

  if(inputSwitch->singleClick())
  {
    if(s->getCurrentValue() == 2)
    {
      s->setCurrentValue(1);
      vhih->updateAttribute(s);
      taskSingle->enableDelayed(500);
    }
  }
  if(inputSwitch->doubleClick())
  {
    if(d->getCurrentValue() == 2)
    {
      d->setCurrentValue(1);
      vhih->updateAttribute(d);
      taskDouble->enableDelayed(500);
    }
  }
  if(inputSwitch->longPress())
  {
    if(l->getCurrentValue() == 0)
    {
      l->setCurrentValue(1);
      vhih->updateAttribute(l);
    }
  }

  if(inputSwitch->released())
  {
    if(l->getCurrentValue() == 1)
    {
      l->setCurrentValue(0);
      vhih->updateAttribute(l);
    }
  }  
}

void loop() {
  ArduinoOTA.handle();
  ts.execute();

  processButton(&input1, i1, i1_double, i1_long, &reset_i1, &reset_i1_double);
  processButton(&input2, i2, i2_double, i2_long, &reset_i2, &reset_i2_double);
  processButton(&input3, i3, i3_double, i3_long, &reset_i3, &reset_i3_double);

  
}