#include <BleKeyboard.h>

#define BUILTIN_LED 2
#define LED_R 17
#define LED_G 21
#define LED_B 19
#define KEYSW_1 33
#define KEYSW_2 25
#define KEYSW_3 26
#define KEYSW_4 27
#define KEYSW_5 23
#define KEYSW_6 22
#define KEYSW_7 18
#define KEYSW_8 16
#define SETTINGSW 32
#define LEDC_BASE_FREQ 12800
#define LEDC_RESOLUTION 8
#define LEDC_CHANNEL_R 0
#define LEDC_CHANNEL_G 1
#define LEDC_CHANNEL_B 2

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

int read_all_sw() {
  unsigned int pushed = 0;
  if(digitalRead(SETTINGSW) == LOW) {
    pushed |= (1<<0);
  }
  if(digitalRead(KEYSW_1) == LOW) {
    pushed |= (1<<1);
  }
  if(digitalRead(KEYSW_2) == LOW) {
    pushed |= (1<<2);
  }
  if(digitalRead(KEYSW_3) == LOW) {
    pushed |= (1<<3);
  }
  if(digitalRead(KEYSW_4) == LOW) {
    pushed |= (1<<4);
  }
  if(digitalRead(KEYSW_5) == LOW) {
    pushed |= (1<<5);
  }
  if(digitalRead(KEYSW_6) == LOW) {
    pushed |= (1<<6);
  }
  if(digitalRead(KEYSW_7) == LOW) {
    pushed |= (1<<7);
  }
  if(digitalRead(KEYSW_8) == LOW) {
    pushed |= (1<<8);
  }
  return pushed;
}

void setup() {
  pinMode(KEYSW_1, INPUT_PULLUP);
  pinMode(KEYSW_2, INPUT_PULLUP);
  pinMode(KEYSW_3, INPUT_PULLUP);
  pinMode(KEYSW_4, INPUT_PULLUP);
  pinMode(KEYSW_5, INPUT_PULLUP);
  pinMode(KEYSW_6, INPUT_PULLUP);
  pinMode(KEYSW_7, INPUT_PULLUP);
  pinMode(KEYSW_8, INPUT_PULLUP);
  pinMode(SETTINGSW, INPUT_PULLUP);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);
  ledcSetup(LEDC_CHANNEL_R, LEDC_BASE_FREQ, LEDC_RESOLUTION);
  ledcAttachPin(LED_R, LEDC_CHANNEL_R);
  ledcSetup(LEDC_CHANNEL_G, LEDC_BASE_FREQ, LEDC_RESOLUTION);
  ledcAttachPin(LED_G, LEDC_CHANNEL_G);
  ledcSetup(LEDC_CHANNEL_B, LEDC_BASE_FREQ, LEDC_RESOLUTION);
  ledcAttachPin(LED_B, LEDC_CHANNEL_B);
  ledcWrite(LEDC_CHANNEL_R, 0);
  ledcWrite(LEDC_CHANNEL_G, 0);
  ledcWrite(LEDC_CHANNEL_B, 0);
  bleKeyboard.begin();
}

void loop() {
  // static variable defind.
  static unsigned int sw_pushed_saved = 0;
  // measures for chattering.
  if(sw_pushed_saved != read_all_sw()) {
    digitalWrite(BUILTIN_LED, HIGH);
    delay(5);
    // for LED
    unsigned int sw_pushed = read_all_sw();
    unsigned int sw_pushed_xor = sw_pushed ^ sw_pushed_saved;
    if(sw_pushed & (1<<1)) {
      PwmLed(LEDC_CHANNEL_R);
    }
    if(sw_pushed & (1<<2)) {
      PwmLed(LEDC_CHANNEL_G);
    }
    if(sw_pushed & (1<<3)) {
      PwmLed(LEDC_CHANNEL_B);
    }
    // setting sw
    if(sw_pushed_xor & (1<<0)) {
      //
    }
    // ble
    if(bleKeyboard.isConnected()) {
      if(sw_pushed_xor & (1<<4)) {
        if(sw_pushed & (1<<4)) {
          bleKeyboard.press(KEY_BACKSPACE);
        } else {
          bleKeyboard.release(KEY_BACKSPACE);
        }
      }
      if(sw_pushed_xor & (1<<5)) {
        if(sw_pushed & (1<<5)) {
          bleKeyboard.press('2');
        } else {
          bleKeyboard.release('2');
        }
      }
      if(sw_pushed_xor & (1<<6)) {
        if(sw_pushed & (1<<6)) {
          bleKeyboard.press('\"');
        } else {
          bleKeyboard.release('\"');
        }
      }
      if(sw_pushed_xor & (1<<7)) {
        if(sw_pushed & (1<<7)) {
          bleKeyboard.press(KEY_LEFT_ALT);
          bleKeyboard.press(KEY_TAB);
        } else {
          bleKeyboard.release(KEY_LEFT_ALT);
          bleKeyboard.release(KEY_TAB);
        }
      }
      if(sw_pushed_xor & (1<<8)) {
        if(sw_pushed & (1<<8)) {
          bleKeyboard.press(KEY_RETURN);
        } else {
          bleKeyboard.release(KEY_RETURN);
        }
      }
    }
    sw_pushed_saved = sw_pushed;
    digitalWrite(BUILTIN_LED, LOW);
  }
}
