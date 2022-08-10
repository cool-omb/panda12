#include <BleKeyboard.h>
#include <vector>
#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>
#include <string>

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

const int CHAR_ARRAY_LENGTH = 64;
const int SSID_LENGTH = 32;
const char* DEFAULT_DEVICE_NAME = "keyboard_panda12";
const char* DEFAULT_DEVICE_MANUFACTURER = "panda12_manufacturer";

const char* DEFAULT_SSID = "Keyboard_config";
const char* DEFAULT_PASSWORD = "password";

const int CURRENT_VERSION = 2;

bool is_config = false;

struct ROM{
  int version;
  char device_name[CHAR_ARRAY_LENGTH];
  char device_manufacturer[CHAR_ARRAY_LENGTH];
  char ssid[SSID_LENGTH];
  char password[CHAR_ARRAY_LENGTH];
  int layers_switch[KEYMAPS_COLUMN_LENGTH];
  char keymaps[KEYMAPS_ROW_LENGTH][KEYMAPS_COLUMN_LENGTH];
};

ROM rom;

// EEPROM から読み込み。保存されていない場合はデフォルト値に。
void load_rom() {
  EEPROM.get<ROM>(0x00, rom);
  if(rom.version != CURRENT_VERSION) {
    for(int i = 0; i < CHAR_ARRAY_LENGTH; ++i) {
      rom.device_name[i] = DEFAULT_DEVICE_NAME[i];
    }
    for(int i = 0; i < CHAR_ARRAY_LENGTH; ++i) {
      rom.device_manufacturer[i] = DEFAULT_DEVICE_MANUFACTURER[i];
    }
    for(int i = 0; i < SSID_LENGTH; ++i) {
      rom.ssid[i] = DEFAULT_SSID[i];
    }
    for(int i = 0; i < CHAR_ARRAY_LENGTH; ++i) {
      rom.password[i] = DEFAULT_PASSWORD[i];
    }
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

string html_head = "\
<!DOCTYPE html>\
<html lang='ja'>\
  <head>\
    <meta charset='UTF-8'>\
    <title>keyboard config</title>\
  <head>\
  <body>\
    <form>\
";

string html_foot = "\
    </form>\
  </body>\
</html>\
";

string get_html_input(char* name, char* value) {
  return "<input type=\"text\" name=\""+(string)name+"\" value=\""+(string)value+"\">";
}

const vector<int> LED_RGB_PINS{25, 32, 33};
const vector<int> LED_RGB_CHANNELS{LEDC_CHANNEL_R, LEDC_CHANNEL_G, LEDC_CHANNEL_B};

BleKeyboard bleKeyboard(rom.device_name, rom.device_manufacturer, 100);
WebServer server(80);

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

void handleRoot() {
  string html = html_head+get_html_input("device_name", rom.device_name)+html_foot;
  server.send(200, "text/html", html.c_str());
}

void handleNotFound() {
  server.send(404, "text/plain", "File Not Found.\n\nlink to\n192.168.4.1\n");
}

void setup_config_server() {
  is_config = true;
  WiFi.softAP(rom.ssid, rom.password);
  delay(100);
  WiFi.softAPConfig(static_ip, static_ip, subnet);
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
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
    server.handleClient();
    
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
