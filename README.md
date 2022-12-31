# panda12

## 概要 Overview

Bluetooth 接続の 12 キーキーボード。

キーマップの変更等がWiFi経由で出来る。

## 使い方 Usage
### キーボード使用時
スライドスイッチで電源を入れ、 `Test_keyboard` にBluetooth接続する。
### キーマップ設定時
設定ボタンを押しながら電源を入れ、 `Preference_keyboard` にWiFi接続する。
ブラウザからキーマップを変更後、 `保存` ボタンをクリックすることで変更が反映される。

※ 有線LANを接続しているデバイスからは、キーマップ変更ページへ自動で遷移しないことがあるので、 `192.168.4.1` へ手動でアクセスしてください。

## 必要要件 Requirement

- KiCad 6.0.10

  - [Kailh socket MX footprint](https://github.com/daprice/keyswitches.pretty)

- [Arduino IDE 2.0.3](https://www.arduino.cc/en/software)
  - [Arduino core for the ESP32 v2.0.4](https://github.com/espressif/arduino-esp32#installation-instructions)
  - [NimBLE-Arduino v1.4.0](https://github.com/h2zero/NimBLE-Arduino)
  - [ESP32-NimBLE-Keyboard v1.4.0](https://github.com/wakwak-koba/ESP32-NimBLE-Keyboard)
  - [arduino-esp8266fs-plugin v0.5.0](https://github.com/esp8266/arduino-esp8266fs-plugin)
  - [Arduino ESP32 filesystem uploader v1.0](https://github.com/me-no-dev/arduino-esp32fs-plugin)
  - [ESP8266_Spiram](https://github.com/Gianbacchio/ESP8266_Spiram)
  - [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
  - [ArduinoJson](https://github.com/bblanchon/ArduinoJson)

## 注意事項 Note
- BLE通信の安定性は上記のライブラリに依存しており、通信するデバイスによっては動作が不安定になることもございます。
- `Blekeyboard.print()` の挙動が不安定で、これを利用しているテキスト出力もまた安定的に動作しません。

## 参考 Reference

- [CaptivePortalExample](https://github.com/elliotmade/ESP32-Captive-Portal-Example/blob/test/src/main.cpp)
