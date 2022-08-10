#include <BleKeyboard.h>
#include <vector>
#include <EEPROM.h>
#include <WiFi.h>

#define LEDC_BASE_FREQ 12800
#define LEDC_RESOLUTION 8
#define LEDC_CHANNEL_R 0
#define LEDC_CHANNEL_G 1
#define LEDC_CHANNEL_B 2

using namespace std;

const int SETTING_SW_PIN = 13;
const vector<int> SW_PINS{16, 19, 23, 14, 4, 18, 22, 27, 15, 17, 21, 26};

const IPAddress static_ip(192, 168, 4, 1);
const IPAddress subnet(255, 255, 255, 0);

const int KEYMAPS_ROW_LENGTH = 4;
const int KEYMAPS_COLUMN_LENGTH = 12;
const int DEFAULT_LAYERS_SWITCH[] = {0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 1};
const char DEFAULT_KEYMAPS[KEYMAPS_ROW_LENGTH][KEYMAPS_COLUMN_LENGTH] = {
  {'1', '2', '3', '0', '4', '5', '6', '\0', '7', '8', '9', '\0'},
  {'+', '-', '*', '/', '%', '(', ')', '\0', KEY_LEFT_ARROW, KEY_RIGHT_ARROW, KEY_BACKSPACE, '\0'},
  {KEY_F1, KEY_F2, KEY_F3, KEY_F10, KEY_F4, KEY_F5, KEY_F6, '\0', KEY_F7, KEY_F8, KEY_F9, '\0'},
  {KEY_F11, KEY_F12, KEY_F13, KEY_F20, KEY_F14, KEY_F15, KEY_F16, '\0', KEY_F17, KEY_F18, KEY_F19, '\0'}
};

const char* DEFAULT_SSID = "Keyboard_config";
const char* DEFAULT_PASSWORD = "password";

const int CURRENT_VERSION = 2;

bool is_config = false;

struct ROM{
  int version;
  int layers_switch[KEYMAPS_COLUMN_LENGTH];
  char keymaps[KEYMAPS_ROW_LENGTH][KEYMAPS_COLUMN_LENGTH];
};

ROM rom;

// EEPROM から読み込み。保存されていない場合はデフォルト値に。
void load_rom() {
  EEPROM.get<ROM>(0x00, rom);
  if(rom.version != CURRENT_VERSION) {
    for(int i = 0; i < KEYMAPS_ROW_LENGTH; ++i) {
      for(int j = 0; j < KEYMAPS_COLUMN_LENGTH; ++j) {
        rom.keymaps[i][j] = DEFAULT_KEYMAPS[i][j];
      }
    }
    for(int i = 0; i < KEYMAPS_COLUMN_LENGTH; ++i) {
      rom.layers_switch[i] = DEFAULT_LAYERS_SWITCH[i];
    }
    rom.version = CURRENT_VERSION;
  }
}

void save_rom() {
  EEPROM.put<ROM>(0x00, rom);
  EEPROM.commit();
}

const vector<int> LED_RGB_PINS{25, 32, 33};
const vector<int> LED_RGB_CHANNELS{LEDC_CHANNEL_R, LEDC_CHANNEL_G, LEDC_CHANNEL_B};

BleKeyboard bleKeyboard("BleMacroKeyboard", "BleMacroKeyboardd_manufacturer", 100);
WiFiServer server(80);

void PwmLed(int channel) {
  static uint8_t brightness[3] = {0, 0, 0};
  static int diff[3] = {1, 1, 1};
  ledcWrite(channel, brightness[channel]);
  if (brightness[channel] == 0) {
    diff[channel] = 1;
  } else if (brightness[channel] == 255) {
    diff[channel] = -1;
  }
  brightness[channel] += diff[channel];
}

unsigned int read_all_sw() {
  unsigned int pushed = 0;
  for(int i = 0; i < (int)SW_PINS.size(); ++ i) {
    if(digitalRead(SW_PINS[i]) == LOW) {
      pushed |= (1<<i);
    }
  }
  return pushed;
}

void setup_blekeyboard() {
  bleKeyboard.begin();
}

void setup_config_server() {
  is_config = true;
  WiFi.softAP(DEFAULT_SSID, DEFAULT_PASSWORD);
  delay(100);
  WiFi.softAPConfig(static_ip, static_ip, subnet);
  server.begin();
}

void setup() {
  // setting_sw pins setup
  pinMode(SETTING_SW_PIN, INPUT_PULLUP);

  // sw pins setup
  for(int pin : SW_PINS) {
    pinMode(pin, INPUT_PULLUP);
  }

  // LED pins setup
  for(int i = 0; i < (int)SW_PINS.size(); ++i) {
    ledcSetup(LED_RGB_CHANNELS[i], LEDC_BASE_FREQ, LEDC_RESOLUTION);
    ledcAttachPin(LED_RGB_PINS[i], LED_RGB_CHANNELS[i]);
    ledcWrite(LED_RGB_CHANNELS[i], 0);
  }
  delay(10);

  // EEPROM setup
  EEPROM.begin(1024);
  load_rom();
  delay(10);

  if(digitalRead(SETTING_SW_PIN) == LOW) {
    setup_config_server();
  } else {
    setup_blekeyboard();
  }
}

void loop() {
  if(is_config) {
    WiFiClient client = server.available();
    if (client) {                             // if you get a client,
      String currentLine = "";                // make a String to hold incoming data from the client
      while (client.connected()) {            // loop while the client's connected
        if (client.available()) {             // if there's bytes to read from the client,
          char c = client.read();             // read a byte, then
          Serial.write(c);                    // print it out the serial monitor
          if (c == '\n') {                    // if the byte is a newline character

            // if the current line is blank, you got two newline characters in a row.
            // that's the end of the client HTTP request, so send a response:
            if (currentLine.length() == 0) {
              // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
              // and a content-type so the client knows what's coming, then a blank line:
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println();

              // the content of the HTTP response follows the header:
              client.print("Click <a href=\"/H\">here</a> to turn the LED on pin 5 on.<br>");
              client.print("Click <a href=\"/L\">here</a> to turn the LED on pin 5 off.<br>");

              // The HTTP response ends with another blank line:
              client.println();
              // break out of the while loop:
              break;
            } else {    // if you got a newline, then clear currentLine:
              currentLine = "";
            }
          } else if (c != '\r') {  // if you got anything else but a carriage return character,
            currentLine += c;      // add it to the end of the currentLine
          }
        }
      }
      // close the connection:
      client.stop();
    }
  } else {
    // static variable defind.
    static unsigned int sw_pushed = 0;
    static int keymap_layer = 0;
    // measures for chattering.
    if(sw_pushed != read_all_sw()) {
      delay(10);

      unsigned int cur_sw_pushed = read_all_sw();
      unsigned int xor_sw_pushed = cur_sw_pushed ^ sw_pushed;
      // for LED
      // if(cur_sw_pushed & (1<<1)) {
      //   PwmLed(LEDC_CHANNEL_R);
      // }
      // if(cur_sw_pushed & (1<<2)) {
      //   PwmLed(LEDC_CHANNEL_G);
      // }
      // if(cur_sw_pushed & (1<<3)) {
      //   PwmLed(LEDC_CHANNEL_B);
      // }
      // ble
      if(bleKeyboard.isConnected()) {
        int cur_keymap_layer = 0;
        for(int i = 0; i < KEYMAPS_COLUMN_LENGTH; ++i) {
          if(cur_sw_pushed & (1<<i)) {
            cur_keymap_layer += rom.layers_switch[i];
          }
        }
        if(keymap_layer != cur_keymap_layer) {
          bleKeyboard.releaseAll();
          for(int i = 0; i < KEYMAPS_COLUMN_LENGTH; ++i) {
            if(rom.keymaps[cur_keymap_layer][i] != '\0') {
              if(cur_sw_pushed & (1<<i)) {
                bleKeyboard.press(rom.keymaps[cur_keymap_layer][i]);
              }
            }
          }
          keymap_layer = cur_keymap_layer;
        } else {
          for(int i = 0; i < (int)SW_PINS.size(); ++i) {
            if(rom.keymaps[keymap_layer][i] == '\0') {
              continue;
            }
            if(xor_sw_pushed & (1<<i)) {
              if(cur_sw_pushed & (1<<i)) {
                bleKeyboard.press(rom.keymaps[keymap_layer][i]);
              } else {
                bleKeyboard.release(rom.keymaps[keymap_layer][i]);
              }
            }
          }
        }
        
      }
      sw_pushed = cur_sw_pushed;
    }
  }
}
