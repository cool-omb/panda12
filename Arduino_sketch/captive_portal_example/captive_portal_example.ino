#include <DNSServer.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include "ESPAsyncWebServer.h"
#include <SPIFFS.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"

DNSServer dnsServer;
AsyncWebServer server(80);

class CaptiveRequestHandler : public AsyncWebHandler {
public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request){
        //request->addInterestingHeader("ANY");
        return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
    }
};


void webServerSetup()
{
    // server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);

    AsyncCallbackJsonWebHandler* jsonHandler = new AsyncCallbackJsonWebHandler(
        "/keymap.json", [](AsyncWebServerRequest *request, JsonVariant &json
        ) {
        // JsonObject jsonObject = json.as<JsonObject>();
        Serial.println("Call AsyncCallbackJsonWebHandler.");
        request->send(200, "application/json", "{test: \"ok\"}");
        }
    );
    server.addHandler(jsonHandler);

    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
    
    // server.on("/keymap.json", HTTP_POST, [](AsyncWebServerRequest* request) {
        // AsyncWebServerResponse* response = request->beginResponse(200, "text/html", "hello world");
        // response->addHeader("Connection", "close");
        // request->send(response);
    //   }, handleUpload);

    // server.onFileUpload(handleUpload);

    server.begin();
}

void setup(){
    Serial.begin(115200);
    SPIFFS.begin();
    WiFi.softAP("esp-captive");
    dnsServer.start(53, "*", WiFi.softAPIP());
    webServerSetup();
}

void loop(){
    dnsServer.processNextRequest();   
}