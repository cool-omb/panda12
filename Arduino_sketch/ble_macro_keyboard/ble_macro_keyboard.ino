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
  static bool keysw_pushed[9] = {
    false, false, false, false, false, false, false, false, false
  };
  // each sw pushed.
  if(digitalRead(SETTINGSW) == LOW) {
    keysw_pushed[0] = true;
  } else {
    keysw_pushed[0] = false;
  }
  if(digitalRead(KEYSW_1) == LOW) {
    PwmLed(LEDC_CHANNEL_R);
    delay(10);
    keysw_pushed[1] = true;
  } else {
    keysw_pushed[1] = false;
  }
  if(digitalRead(KEYSW_2) == LOW) {
    PwmLed(LEDC_CHANNEL_G);
    delay(10);
    keysw_pushed[2] = true;
  } else {
    keysw_pushed[2] = false;
  }
  if(digitalRead(KEYSW_3) == LOW) {
    PwmLed(LEDC_CHANNEL_B);
    delay(10);
    keysw_pushed[3] = true;
  } else {
    keysw_pushed[3] = false;
  }
  if(bleKeyboard.isConnected()) {
    if(digitalRead(KEYSW_4) == LOW) {
      if(!keysw_pushed[4]) {
        bleKeyboard.press('a');
        delay(10);
      }
      keysw_pushed[4] = true;
    } else {
      if(keysw_pushed[4]) {
        bleKeyboard.release('a');
        delay(10);
      }
      keysw_pushed[4] = false;
    }
    if(digitalRead(KEYSW_5) == LOW) {
      if(!keysw_pushed[5]) {
        bleKeyboard.press(KEY_LEFT_CTRL);
        bleKeyboard.press('c');
        delay(10);
      }
      keysw_pushed[5] = true;
    } else {
      if(keysw_pushed[5]) {
        bleKeyboard.release(KEY_LEFT_CTRL);
        bleKeyboard.release('c');
        delay(10);
      }
      keysw_pushed[5] = false;
    }
    if(digitalRead(KEYSW_6) == LOW) {
      if(!keysw_pushed[6]) {
        bleKeyboard.press(KEY_LEFT_CTRL);
        bleKeyboard.press('v');
        delay(10);
      }
      keysw_pushed[6] = true;
    } else {
      if(keysw_pushed[6]) {
        bleKeyboard.release(KEY_LEFT_CTRL);
        bleKeyboard.release('v');
        delay(10);
      }
      keysw_pushed[6] = false;
    }
    if(digitalRead(KEYSW_7) == LOW) {
      if(!keysw_pushed[7]) {
        bleKeyboard.print("Alltheworldisastage");
        delay(10);
      }
      keysw_pushed[7] = true;
    } else {
      keysw_pushed[7] = false;
    }
    if(digitalRead(KEYSW_8) == LOW) {
      if(!keysw_pushed[8]) {
        bleKeyboard.print("ta_toshiki_kura@nnn.ac.jp");
        delay(10);
      }
      keysw_pushed[8] = true;
    } else {
      keysw_pushed[8] = false;
    }
    digitalWrite(BUILTIN_LED, LOW);
    for(const auto& pushed : keysw_pushed) {
      if(pushed) {
        digitalWrite(BUILTIN_LED, HIGH);
        break;
      }
    }
  }
}
