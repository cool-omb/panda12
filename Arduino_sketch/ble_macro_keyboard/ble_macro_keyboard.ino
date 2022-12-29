#include <Arduino.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "DNSServer.h"
#include <SPIFFS.h>
#include "ArduinoJson.h"
#include <FS.h>
#include <BleKeyboard.h>

using namespace std;

#define LED_R_PIN 25
#define LED_G_PIN 32
#define LED_B_PIN 33
#define LEDC_BASE_FREQ 12800
#define LEDC_RESOLUTION 8
#define LEDC_CHANNEL_R 0
#define LEDC_CHANNEL_G 1
#define LEDC_CHANNEL_B 2


#define PREFERENCE_SW_PIN 13
#define DEFAULT_DEVICE_NAME "Test_Keyboard"
#define DEFAULT_DEVICE_MANUFACTURER "Panda12_manufacturer"

bool isConfig = false;
const vector<int> SW_PINS{16, 19, 23, 14, 4, 18, 22, 27, 15, 17, 21, 26};

const char *SSID = "Preference_Keyboard";

int LAYER_LENGTH = -1;
int OUTPUT_LENGTH = -1;
int INPUT_LENGTH = -1;
int INTERVAL_MS = 100;
int IS_LED_ON = 0;
int LED_R_BRIGHTNESS = 0;
int LED_G_BRIGHTNESS = 0;
int LED_B_BRIGHTNESS = 0;

AsyncWebServer server(80);
DNSServer dnsServer;

BleKeyboard bleKeyboard(DEFAULT_DEVICE_NAME, DEFAULT_DEVICE_MANUFACTURER, 100);
DynamicJsonDocument preferenceDocument(65536);

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
  if(IS_LED_ON) {
    ledcSetup(LEDC_CHANNEL_R, LEDC_BASE_FREQ, LEDC_RESOLUTION);
    ledcAttachPin(LED_R_PIN, LEDC_CHANNEL_R);
    ledcWrite(LEDC_CHANNEL_R, LED_R_BRIGHTNESS);
    ledcSetup(LEDC_CHANNEL_G, LEDC_BASE_FREQ, LEDC_RESOLUTION);
    ledcAttachPin(LED_G_PIN, LEDC_CHANNEL_G);
    ledcWrite(LEDC_CHANNEL_G, LED_G_BRIGHTNESS);
    ledcSetup(LEDC_CHANNEL_B, LEDC_BASE_FREQ, LEDC_RESOLUTION);
    ledcAttachPin(LED_B_PIN, LEDC_CHANNEL_B);
    ledcWrite(LEDC_CHANNEL_B, LED_B_BRIGHTNESS);
  }
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
  File file = SPIFFS.open("/keymap.json", "r");
  DeserializationError error = deserializeJson(preferenceDocument, file);
  if (error) {
    // Serial.println(F("Error: deserializeJson"));
    // Serial.println(error.c_str());
  }
  JsonObject root = preferenceDocument.as<JsonObject>();

  // Serial.println(preferenceDocument["version"].as<String>());
  // Serial.println(preferenceDocument["layout"][0].as<String>());
  file.close();

  LAYER_LENGTH = preferenceDocument["length"]["layer"].as<int>();
  OUTPUT_LENGTH = preferenceDocument["length"]["output"].as<int>();
  INPUT_LENGTH = preferenceDocument["length"]["input"].as<int>();
  INTERVAL_MS = preferenceDocument["preference"]["interval"].as<int>();
  IS_LED_ON = preferenceDocument["preference"]["ledOn"].as<int>();
  LED_R_BRIGHTNESS = preferenceDocument["preference"]["ledRed"].as<int>();
  LED_G_BRIGHTNESS = preferenceDocument["preference"]["ledGreen"].as<int>();
  LED_B_BRIGHTNESS = preferenceDocument["preference"]["ledBlue"].as<int>();

  setupLed();

  delay(20);
  // Serial.println(INPUT_LENGTH);
  bleKeyboard.begin();
  delay(100);
}

unsigned int readAllPushedBit() {
  unsigned int pushed = 0;
  for(int i = 0; i < (int)SW_PINS.size(); ++ i) {
    if(digitalRead(SW_PINS[i]) == LOW) {
      pushed |= (1<<i);
    }
  }
  return pushed;
}

int getKeyNumber(int layerIndex, int outputIndex, int inputIndex) {
  return preferenceDocument["assign"][layerIndex][outputIndex][inputIndex]["num"].as<int>();
}

String getKeyText(int layerIndex, int outputIndex, int inputIndex) {
  return preferenceDocument["assign"][layerIndex][outputIndex][inputIndex]["text"].as<String>();
}

void loopKeyboard() {
  // static variable defind.
  static unsigned int swPushedBit = 0;
  static int layerIndex = 0;

  // measures for chattering.
  if(swPushedBit != readAllPushedBit()) {
    delay(10);
    if(bleKeyboard.isConnected()) {
      unsigned int currentSwPushedBit = readAllPushedBit();
      unsigned int xorSwPushedBit = currentSwPushedBit ^ swPushedBit;

      int currentLayerIndex = 0;
      for(int i = 0; i < INPUT_LENGTH; ++i) {
        if(currentSwPushedBit & (1<<i)) {
          if(getKeyNumber(0, 0 , i) >= 256) {
            currentLayerIndex += getKeyNumber(0, 0 , i) - 256;
          }
          
        }
      }
      if(layerIndex != currentLayerIndex) {
        bleKeyboard.releaseAll();
        for(int i = 0; i < INPUT_LENGTH; ++i) {
          if (currentSwPushedBit & (1<<i)) {
            int keyNumber = getKeyNumber(currentLayerIndex, 0, i);
            if (keyNumber < 0) {
              // Serial.println(getKeyText(currentLayerIndex, 0, i));
              bleKeyboard.print(getKeyText(currentLayerIndex, 0, i));
            } else if (keyNumber < 256) {
              // Serial.println(keyNumber);
              bleKeyboard.press(keyNumber);
            }
          }
        }
        layerIndex = currentLayerIndex;
      } else {
        for(int i = 0; i < (int)SW_PINS.size(); ++i) {
          if(xorSwPushedBit & (1<<i)) {
            int keyNumber = getKeyNumber(currentLayerIndex, 0, i);
            if(currentSwPushedBit & (1<<i)) {
              if (keyNumber < 0) {
                // Serial.println(getKeyText(currentLayerIndex, 0, i));
                bleKeyboard.print(getKeyText(currentLayerIndex, 0, i));
              } else if (keyNumber < 256) {
                // Serial.println(keyNumber);
                bleKeyboard.press(keyNumber);
              }
            } else {
              if(0 <= keyNumber && keyNumber < 256) {
                bleKeyboard.release(keyNumber);
              }
            }
          }
        }
      }
      swPushedBit = currentSwPushedBit;
    }
  }
  delay(INTERVAL_MS);
}

void setup()
{
  setupLed();

  // Serial.begin(115200);

  SPIFFS.begin();

  setupPinMode();
  if (digitalRead(PREFERENCE_SW_PIN) == LOW) {
    isConfig = true;
    setupPreferenceMode();
  } else {
    setupKeyboardMode();
  }
}

void loop()
{
  if (isConfig) {
    dnsServer.processNextRequest();
  } else {
    loopKeyboard();
  }
}