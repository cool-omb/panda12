#include <Arduino.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "DNSServer.h"
#include <SPIFFS.h>

const char *SSID = "Preference_Keyboard";

AsyncWebServer server(80);

DNSServer dnsServer;

void webServerSetup()
{

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html", "text/html"); });
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/style.css", "text/css"); });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/script.js", "text/javascript"); });
    server.on("/keymap.json", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/keymap.json", "application/json"); });
    server.on("/metakey.json", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/metakey.json", "application/json"); });

    server.onNotFound([](AsyncWebServerRequest *request)
                      {
                          String redirectUrl = "http://";
                          redirectUrl += WiFi.softAPIP().toString();
                          redirectUrl += "/";
                          request->redirect(redirectUrl); });

    server.begin();
}

void setup()
{
    SPIFFS.begin();

    WiFi.softAP(SSID);

    dnsServer.start(53, "*", WiFi.softAPIP());

    webServerSetup();
}

void loop()
{
    dnsServer.processNextRequest();
}