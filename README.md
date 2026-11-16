<div align="center">
  <h1>ESP32-C6-Touch-LCD-2.73</h1>
  <p><strong>ESP32-C6 development board with a 2.73-inch 320 × 320 touch LCD</strong></p>
  <p><strong>English</strong> | <a href="README_ZH.md">简体中文</a></p>
  <p><a href="firmware/">📦 Factory Firmware</a> &middot; <a href="examples/factory/">🧩 Factory Source</a></p>
</div>

---

## ✨ Overview

This repository provides the factory demonstration source and ready-to-flash firmware for ESP32-C6-Touch-LCD-2.73. The touch desktop includes display, touch, sensor and GPIO tests, a clock, a photo badge and games.

| Feature | Device / interface |
| --- | --- |
| MCU | ESP32-C6 |
| Memory | 16 MB Flash; no PSRAM |
| Display | 2.73-inch 320 × 320 LCD, ILI9488 |
| Touch | GT911, I²C |
| Sensors | QMI8658 IMU, SHTC3 temperature/humidity |
| Clock | PCF85063A RTC |
| Wireless | 2.4 GHz Wi-Fi used by the factory application |

## 📦 Quick Start

1. Download the newest `FactoryOnly` BIN from [firmware](firmware/).
2. Connect the board by USB and follow the [firmware flashing instructions](firmware/README.md).
3. Flash the merged BIN at **0x0**, then reset the board to open the desktop.

## 🧪 Examples

| Project | Main feature |
| --- | --- |
| [Factory application](examples/factory/) | Eleven apps: Display Test, Settings, Touch Test, Ball, Weather, Sensor, 2048, Button Test, PinsTest, Badge and Watch. |

The Photo Badge stores images in on-board Flash and accepts uploads from a phone. The SD-card Photos gallery is not included. See the [factory guide](examples/factory/README.md) for controls and Wi-Fi setup.

## 🛠️ Development Environment

| Framework | Version | Projects |
| --- | --- | ---: |
| ESP-IDF | v5.5.4 | 1 factory application |
| LVGL | 9.5.0 | GUI dependency |

Open [examples/factory](examples/factory/) in an ESP-IDF terminal and run `idf.py build`. Dependencies are downloaded automatically by the ESP-IDF Component Manager.

## 📚 Resources

- [Factory application guide](examples/factory/README.md)
- [Factory firmware and flashing](firmware/README.md)
- [Board support and pin definitions](examples/factory/components/esp32_c6_touch_lcd_2_73/include/bsp/esp32_c6_touch_lcd_2_73.h)

## 🤝 Support

For product usage, hardware or order questions, contact [Waveshare support](https://service.waveshare.com/).

## 📄 License

Apache License 2.0. See [LICENSE](LICENSE); bundled third-party components retain their own license notices.
