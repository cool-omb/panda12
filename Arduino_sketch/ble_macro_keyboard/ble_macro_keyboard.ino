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

const vector<int> SW_PINS{13, 16, 19, 23, 14, 4, 18, 22, 27, 15, 17, 21, 26};

const int SW_KEYS_LENGTH = 13;
const char DEFALUT_SW_ASSIGN_KEYS[] = {KEY_ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'a', 'b'};

const int CURRENT_VERSION = 2;

struct ROM{
  int version;
  char sw_assign_keys[13];
};

ROM rom;

// EEPROM から読み込み。保存されていない場合はデフォルト値に。
void load_rom() {
  EEPROM.get<ROM>(0x00, rom);
  if(rom.version != CURRENT_VERSION) {
    for(int i = 0; i < SW_KEYS_LENGTH; ++i) {
      rom.sw_assign_keys[i] = DEFALUT_SW_ASSIGN_KEYS[i];
    }
    rom.version = CURRENT_VERSION;
  }
}

void save_rom() {
  rom.sw_assign_keys[12] = 'c';
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
  static unsigned int sw_pushed_saved = 0;
  // measures for chattering.
  if(sw_pushed_saved != read_all_sw()) {
    digitalWrite(BUILTIN_LED, HIGH);
    delay(5);

    unsigned int sw_pushed = read_all_sw();
    unsigned int sw_pushed_xor = sw_pushed ^ sw_pushed_saved;
    // for LED
    // if(sw_pushed & (1<<1)) {
    //   PwmLed(LEDC_CHANNEL_R);
    // }
    // if(sw_pushed & (1<<2)) {
    //   PwmLed(LEDC_CHANNEL_G);
    // }
    // if(sw_pushed & (1<<3)) {
    //   PwmLed(LEDC_CHANNEL_B);
    // }
    // ble
    if(bleKeyboard.isConnected()) {
      for(int i = 0; i < (int)SW_PINS.size(); ++i) {
        if(sw_pushed & (1<<i)) {
          bleKeyboard.press(rom.sw_assign_keys[i]);
          // bleKeyboard.press(DEFALUT_SW_ASSIGN_KEYS[i]);
        } else {
          bleKeyboard.release(rom.sw_assign_keys[i]);
          // bleKeyboard.release(DEFALUT_SW_ASSIGN_KEYS[i]);
        }
      }
    }
    sw_pushed_saved = sw_pushed;
    digitalWrite(BUILTIN_LED, LOW);
  }
}
