#include <BleKeyboard.h>

#define BUILTIN_LED 2
#define LED_R 25
#define LED_G 32
#define LED_B 33
#define KEYSW_01 16
#define KEYSW_02 19
#define KEYSW_03 23
#define KEYSW_04 14
#define KEYSW_05 4
#define KEYSW_06 18
#define KEYSW_07 22
#define KEYSW_08 27
#define KEYSW_09 15
#define KEYSW_10 17
#define KEYSW_11 21
#define KEYSW_12 26

#define SETTINGSW 13
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
  if(digitalRead(KEYSW_01) == LOW) {
    pushed |= (1<<1);
  }
  if(digitalRead(KEYSW_02) == LOW) {
    pushed |= (1<<2);
  }
  if(digitalRead(KEYSW_03) == LOW) {
    pushed |= (1<<3);
  }
  if(digitalRead(KEYSW_04) == LOW) {
    pushed |= (1<<4);
  }
  if(digitalRead(KEYSW_05) == LOW) {
    pushed |= (1<<5);
  }
  if(digitalRead(KEYSW_06) == LOW) {
    pushed |= (1<<6);
  }
  if(digitalRead(KEYSW_07) == LOW) {
    pushed |= (1<<7);
  }
  if(digitalRead(KEYSW_08) == LOW) {
    pushed |= (1<<8);
  }
  if(digitalRead(KEYSW_09) == LOW) {
    pushed |= (1<<9);
  }
  if(digitalRead(KEYSW_10) == LOW) {
    pushed |= (1<<10);
  }
  if(digitalRead(KEYSW_11) == LOW) {
    pushed |= (1<<11);
  }
  if(digitalRead(KEYSW_12) == LOW) {
    pushed |= (1<<12);
  }
  return pushed;
}

void setup() {
  pinMode(KEYSW_01, INPUT_PULLUP);
  pinMode(KEYSW_02, INPUT_PULLUP);
  pinMode(KEYSW_03, INPUT_PULLUP);
  pinMode(KEYSW_04, INPUT_PULLUP);
  pinMode(KEYSW_05, INPUT_PULLUP);
  pinMode(KEYSW_06, INPUT_PULLUP);
  pinMode(KEYSW_07, INPUT_PULLUP);
  pinMode(KEYSW_08, INPUT_PULLUP);
  pinMode(KEYSW_09, INPUT_PULLUP);
  pinMode(KEYSW_10, INPUT_PULLUP);
  pinMode(KEYSW_11, INPUT_PULLUP);
  pinMode(KEYSW_12, INPUT_PULLUP);
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
    // if(sw_pushed & (1<<1)) {
    //   PwmLed(LEDC_CHANNEL_R);
    // }
    // if(sw_pushed & (1<<2)) {
    //   PwmLed(LEDC_CHANNEL_G);
    // }
    // if(sw_pushed & (1<<3)) {
    //   PwmLed(LEDC_CHANNEL_B);
    // }
    // setting sw
    if(sw_pushed_xor & (1<<0)) {
      //
    }
    // ble
    if(bleKeyboard.isConnected()) {
      if(sw_pushed_xor & (1<<1)) {
        if(sw_pushed & (1<<1)) {
          bleKeyboard.press('1');
        } else {
          bleKeyboard.release('1');
        }
      }
      if(sw_pushed_xor & (1<<2)) {
        if(sw_pushed & (1<<2)) {
          bleKeyboard.press('2');
        } else {
          bleKeyboard.release('2');
        }
      }
      if(sw_pushed_xor & (1<<3)) {
        if(sw_pushed & (1<<3)) {
          bleKeyboard.press('3');
        } else {
          bleKeyboard.release('3');
        }
      }
      if(sw_pushed_xor & (1<<4)) {
        if(sw_pushed & (1<<4)) {
          bleKeyboard.press('4');
        } else {
          bleKeyboard.release('4');
        }
      }
      if(sw_pushed_xor & (1<<5)) {
        if(sw_pushed & (1<<5)) {
          bleKeyboard.press('5');
        } else {
          bleKeyboard.release('5');
        }
      }
      if(sw_pushed_xor & (1<<6)) {
        if(sw_pushed & (1<<6)) {
          bleKeyboard.press('6');
        } else {
          bleKeyboard.release('6');
        }
      }
      if(sw_pushed_xor & (1<<7)) {
        if(sw_pushed & (1<<7)) {
          bleKeyboard.press('7');
        } else {
          bleKeyboard.release('7');
        }
      }
      if(sw_pushed_xor & (1<<8)) {
        if(sw_pushed & (1<<8)) {
          bleKeyboard.press('8');
        } else {
          bleKeyboard.release('8');
        }
      }
      if(sw_pushed_xor & (1<<9)) {
        if(sw_pushed & (1<<9)) {
          bleKeyboard.press('9');
        } else {
          bleKeyboard.release('9');
        }
      }
      if(sw_pushed_xor & (1<<10)) {
        if(sw_pushed & (1<<10)) {
          bleKeyboard.press('0');
        } else {
          bleKeyboard.release('0');
        }
      }
      if(sw_pushed_xor & (1<<11)) {
        if(sw_pushed & (1<<11)) {
          bleKeyboard.press('a');
        } else {
          bleKeyboard.release('a');
        }
      }
      if(sw_pushed_xor & (1<<12)) {
        if(sw_pushed & (1<<12)) {
          bleKeyboard.press('b');
        } else {
          bleKeyboard.release('b');
        }
      }
    }
    sw_pushed_saved = sw_pushed;
    digitalWrite(BUILTIN_LED, LOW);
  }
}
