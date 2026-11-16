<div align="center">
  <h1>ESP32-C6-Touch-LCD-2.73</h1>
  <p><strong>ESP32-C6 development board with a 2.73-inch 320 × 320 touch LCD</strong></p>
  <p><strong>English</strong> | <a href="README_ZH.md">简体中文</a></p>
  <p><a href="firmware/">📦 Factory Firmware</a> &middot; <a href="examples/factory/">🧩 Factory Source</a></p>
</div>

---

## ✨ Overview

This repository provides ESP-IDF and Arduino examples, the factory demonstration source and ready-to-flash firmware for ESP32-C6-Touch-LCD-2.73. The touch desktop includes display, touch, sensor and GPIO tests, a clock, a photo badge and games.

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

### Standalone Examples

| Example | Function |
| --- | --- |
| [IDF 01_exio](examples/esp-idf/01_exio/README.md) | 11 touchscreen-controlled EXIO outputs |
| [IDF 02_I2C_qmi8658](examples/esp-idf/02_I2C_qmi8658/README.md) | Six-axis live LCD dashboard |
| [IDF 03_SD_Card](examples/esp-idf/03_SD_Card/README.md) | LCD capacity, free space and file verification |
| [IDF 04_I2C_pcf85063](examples/esp-idf/04_I2C_pcf85063/README.md) | RTC clock, date and weekday on LCD; Wi-Fi Beijing time synchronization |
| [IDF 05_shtc3](examples/esp-idf/05_shtc3/README.md) | Live temperature and humidity cards |
| [IDF 06_lvgl_demo](examples/esp-idf/06_lvgl_demo/README.md) | LVGL display and touch interface |
| [IDF 07_battery_voltage](examples/esp-idf/07_battery_voltage/README.md) | LCD voltage and ADC dashboard |
| [Arduino 01_exio](examples/arduino/examples/01_exio/README.md) | 11 touchscreen-controlled EXIO outputs |
| [Arduino 02_I2C_qmi8658](examples/arduino/examples/02_I2C_qmi8658/README.md) | Six-axis live LCD dashboard |
| [Arduino 03_SD_Card](examples/arduino/examples/03_SD_Card/README.md) | LCD capacity, free space and file verification |
| [Arduino 04_I2C_pcf85063](examples/arduino/examples/04_I2C_pcf85063/README.md) | RTC clock, date and weekday on LCD; Wi-Fi Beijing time synchronization |
| [Arduino 05_shtc3](examples/arduino/examples/05_shtc3/README.md) | Live temperature and humidity cards |
| [Arduino 06_lvgl_demo](examples/arduino/examples/06_lvgl_demo/README.md) | LVGL display and touch interface |
| [Arduino 07_battery_voltage](examples/arduino/examples/07_battery_voltage/README.md) | LCD voltage and ADC dashboard |

## 🛠️ Development Environment

| Framework / library | Version | Use |
| --- | --- | --- |
| ESP-IDF | v5.5.4 | 7 standalone examples and factory application |
| Arduino-ESP32 | 3.3.12 | 7 standalone examples |
| LVGL (IDF / Arduino) | 9.5.0 / 8.4.0 | Display and touch interfaces |
| SensorLib (Arduino) | 0.3.1 | QMI8658 |

Start with the [ESP-IDF guide](docs/getting-started-idf.md) or [Arduino guide](docs/getting-started-arduino.md), then follow the selected example README. Arduino uses **16 MB Flash, DIO, 16M Flash (3MB APP/9.9MB FATFS), USB CDC On Boot Enabled**. No PSRAM setting is required.

## 📚 Resources

- [Factory application guide](examples/factory/README.md)
- [Factory firmware and flashing](firmware/README.md)
- [Board support and pin definitions](examples/factory/components/esp32_c6_touch_lcd_2_73/include/bsp/esp32_c6_touch_lcd_2_73.h)
- [Board IO and display format](docs/board-io.md)
- [Baseboard schematic](hardware/schematic/ESP32-C6-LCD-2.73.pdf)

## 🤝 Support

For product usage, hardware or order questions, contact [Waveshare support](https://service.waveshare.com/).

## 📄 License

Apache License 2.0. See [LICENSE](LICENSE); bundled third-party components retain their own license notices.
