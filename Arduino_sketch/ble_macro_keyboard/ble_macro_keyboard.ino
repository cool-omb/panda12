#include <BleKeyboard.h>
#include <vector>

#define BUILTIN_LED 2

#define LEDC_BASE_FREQ 12800
#define LEDC_RESOLUTION 8
#define LEDC_CHANNEL_R 0
#define LEDC_CHANNEL_G 1
#define LEDC_CHANNEL_B 2

const std::vector<int> SW_PINS{13, 16, 19, 23, 14, 4, 18, 22, 27, 15, 17, 21, 26};
std::vector<char> sw_assign_keys{KEY_ESC, '2', '1', '3', '4', '5', '6', '7', '8', '9', '0', 'a', 'b'};

const std::vector<int> LED_RGB_PINS{25, 32, 33};
const std::vector<int> LED_RGB_CHANNELS{LEDC_CHANNEL_R, LEDC_CHANNEL_G, LEDC_CHANNEL_B};

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
          bleKeyboard.press(sw_assign_keys[i]);
        } else {
          bleKeyboard.release(sw_assign_keys[i]);
        }
      }
    }
    sw_pushed_saved = sw_pushed;
    digitalWrite(BUILTIN_LED, LOW);
  }
}
