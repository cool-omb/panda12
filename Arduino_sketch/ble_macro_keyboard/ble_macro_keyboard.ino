#include <BleKeyboard.h>
#include <vector>

#include <EEPROM.h>

#define BUILTIN_LED 2

#define LEDC_BASE_FREQ 12800
#define LEDC_RESOLUTION 8
#define LEDC_CHANNEL_R 0
#define LEDC_CHANNEL_G 1
#define LEDC_CHANNEL_B 2

using namespace std;

const int SETTING_SW_PIN = 13;
const vector<int> SW_PINS{16, 19, 23, 14, 4, 18, 22, 27, 15, 17, 21, 26};

const int KEYMAPS_ROW_LENGTH = 4;
const int KEYMAPS_COLUMN_LENGTH = 12;
const int DEFAULT_LAYERS_SWITCH[] = {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2};
const char DEFAULT_KEYMAPS[KEYMAPS_ROW_LENGTH][KEYMAPS_COLUMN_LENGTH] = {
  {'1', '2', '3', '4', '5', '6', '7', '\0', '9', '0', 'a', '\0'},
  {'d', 'e', 'f', 'g', 'h', 'i', 'j', '\0', 'l', 'm', 'n', '\0'},
  {'x', 'e', 'f', 'g', 'h', 'i', 'j', '\0', 'l', 'm', 'n', '\0'},
  {'y', 'e', 'f', 'g', 'h', 'i', 'j', '\0', 'l', 'm', 'n', '\0'}
};

const int CURRENT_VERSION = 2;

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

void setup() {
  // EEPROM setup
  EEPROM.begin(1024);
  load_rom();
  // save_rom();

  // sw pins setup
  for(int pin : SW_PINS) {
    pinMode(pin, INPUT_PULLUP);
  }

  // Builtin LED setup
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);

  // LED pins setup
  for(int i = 0; i < (int)SW_PINS.size(); ++i) {
    ledcSetup(LED_RGB_CHANNELS[i], LEDC_BASE_FREQ, LEDC_RESOLUTION);
    ledcAttachPin(LED_RGB_PINS[i], LED_RGB_CHANNELS[i]);
    ledcWrite(LED_RGB_CHANNELS[i], 0);
  }
  bleKeyboard.begin();
}

void loop() {
  // static variable defind.
  static unsigned int sw_pushed = 0;
  static int keymap_layer = 0;
  // measures for chattering.
  if(sw_pushed != read_all_sw()) {
    digitalWrite(BUILTIN_LED, HIGH);
    delay(5);

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
        for(int i = 0; i < KEYMAPS_COLUMN_LENGTH; ++i) {
          if(rom.keymaps[keymap_layer][i] != '\0') {
            if(sw_pushed & (1<<i)) {
              bleKeyboard.release(rom.keymaps[keymap_layer][i]);
            }
          }
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
    digitalWrite(BUILTIN_LED, LOW);
  }
}
