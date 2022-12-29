#include <Arduino.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "DNSServer.h"
#include <SPIFFS.h>
#include "ArduinoJson.h"
#include <FS.h>

using namespace std;

#define LED_R_PIN 27
#define LED_G_PIN 25
#define LED_B_PIN 26

#define PREFERENCE_SW_PIN 13

bool is_config = false;
const vector<int> SW_PINS{16, 19, 23, 14, 4, 18, 22, 27, 15, 17, 21, 26};

const char *SSID = "Preference_Keyboard";

AsyncWebServer server(80);
DNSServer dnsServer;

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
  // if (!index) {
  //     request->_tempFile = SPIFFS.open("/" + filename, "w");
  // }
  // if (len) {
  //     request->_tempFile.write(data, len);
  // }
  // if (final) {
  //     request->_tempFile.close();
  //     request->send(200, "text/plain", "filename: "+String(filename));
  // }
}

void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  if(!index){
    // Serial.printf("BodyStart: %u B\n", total);
    request->_tempFile = SPIFFS.open("/keymap.json", "w");
  }
  request->_tempFile.write(data, len);
  // Serial.printf("%s", (const char*)data);
  if(index + len == total){
    request->_tempFile.close();
    request->send(200, "text/plain", "Write keymap.json success.");
    // Serial.printf("BodyEnd: %u B\n", total);
  }
}

void webServerSetup()
{
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

  server.onNotFound([](AsyncWebServerRequest *request)
  {
    String redirectUrl = "http://";
    redirectUrl += WiFi.softAPIP().toString();
    redirectUrl += "/";
    request->redirect(redirectUrl);
  });

  server.begin();
}


void setupLed() {
  pinMode(LED_R_PIN, OUTPUT);
  pinMode(LED_G_PIN, OUTPUT);
  pinMode(LED_B_PIN, OUTPUT);
  digitalWrite(LED_R_PIN, LOW);
  digitalWrite(LED_G_PIN, LOW);
  digitalWrite(LED_B_PIN, LOW);
}

void setupPinMode() {
  pinMode(PREFERENCE_SW_PIN, INPUT_PULLUP);
  for(int pin : SW_PINS) {
    pinMode(pin, INPUT_PULLUP);
  }
}

void setupPreferenceMode() {
  WiFi.softAP(SSID);
  dnsServer.start(53, "*", WiFi.softAPIP());
  webServerSetup();
}

void setupKeyboardMode() {

}

void setup()
{
  setupLed();

  Serial.begin(115200);

  SPIFFS.begin();

  setupPinMode();
  if (digitalRead(PREFERENCE_SW_PIN) == LOW) {
    setupPreferenceMode();
  } else {
    setupKeyboardMode();
  }
}

void loop()
{
  dnsServer.processNextRequest();
}