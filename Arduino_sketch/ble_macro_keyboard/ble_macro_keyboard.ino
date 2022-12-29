#include <Arduino.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "DNSServer.h"
#include <SPIFFS.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include <FS.h>

#define LED_R_PIN 27
#define LED_G_PIN 25
#define LED_B_PIN 26

const char *SSID = "Preference_Keyboard";

AsyncWebServer server(80);

DNSServer dnsServer;

StaticJsonDocument<int(pow(2,16))> jsonDocument;

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
  if (!index) {
      request->_tempFile = SPIFFS.open("/" + filename, "w");
  }
  if (len) {
      request->_tempFile.write(data, len);
  }
  if (final) {
      request->_tempFile.close();
      request->send(200, "text/plain", "filename: "+String(filename));
  }
}

void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  DynamicJsonDocument jsonResponse(1024);
  String res = "";
  if(!index){
    Serial.printf("BodyStart: %u B\n", total);
    request->_tempFile = SPIFFS.open("/keymap.json", "w");
  }
  request->_tempFile.write(data, len);
  Serial.printf("%s", (const char*)data);
  if(index + len == total){
    request->_tempFile.close();
    request->send(200, "text/plain", "/keymap.json");
    Serial.printf("BodyEnd: %u B\n", total);
  }
  // request->send(200, "text/plain", res);
}

void webServerSetup()
{
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
  server.on(
    "/post",
    HTTP_POST,
    [](AsyncWebServerRequest* request){
      request->send(200, "text/plain", "keymap.json posted.");
    },
    handleUpload,
    handleBody
  );
  
  // server.on("/keymap.json", HTTP_POST, [](AsyncWebServerRequest* request) {
    // AsyncWebServerResponse* response = request->beginResponse(200, "text/html", "hello world");
    // response->addHeader("Connection", "close");
    // request->send(response);
  //   }, handleUpload);

  // server.onFileUpload(handleUpload);

  server.onNotFound([](AsyncWebServerRequest *request)
  {
    String redirectUrl = "http://";
    redirectUrl += WiFi.softAPIP().toString();
    redirectUrl += "/";
    request->redirect(redirectUrl);
  });

  server.begin();
}

void setJson(AsyncWebServerRequest *request) {
  request->send(200, "text/plain", String(request->arg("body")));
  // if (request->hasParam("body", true)) {
  //   request->send(200, "text/plain", String(request->getParam("body", true)->value()));
  // } else {
  //   request->send(200, "text/plain", "Not found body param.");
  // }
}



void setupLed() {
  pinMode(LED_R_PIN, OUTPUT);
  pinMode(LED_G_PIN, OUTPUT);
  pinMode(LED_B_PIN, OUTPUT);
  digitalWrite(LED_R_PIN, LOW);
  digitalWrite(LED_G_PIN, LOW);
  digitalWrite(LED_B_PIN, LOW);
}

void setup()
{
  setupLed();

  Serial.begin(115200);

  SPIFFS.begin();

  WiFi.softAP(SSID);

  dnsServer.start(53, "*", WiFi.softAPIP());

  webServerSetup();
}

void loop()
{
  dnsServer.processNextRequest();
}