<div align="center">
  <h1>ESP32-C6-Touch-LCD-2.73</h1>
  <p><strong>搭载 2.73 英寸 320 × 320 触摸液晶屏的 ESP32-C6 开发板</strong></p>
  <p><a href="README.md">English</a> | <strong>简体中文</strong></p>
  <p><a href="firmware/">📦 出厂固件</a> &middot; <a href="examples/factory/">🧩 出厂源码</a></p>
</div>

---

## ✨ 概述

本仓库提供 ESP-IDF 和 Arduino 外设示例，以及 ESP32-C6-Touch-LCD-2.73 的出厂演示源码与可直接烧录的合并固件。触摸桌面包含显示、触摸、传感器和 GPIO 测试，以及电子时钟、电子徽章和游戏。

| 功能 | 器件或接口 |
| --- | --- |
| MCU | ESP32-C6 |
| 存储器 | 16 MB Flash，无 PSRAM |
| 显示 | 2.73 英寸 320 × 320 液晶屏，ILI9488 |
| 触摸 | GT911，I²C |
| 传感器 | QMI8658 惯性传感器、SHTC3 温湿度传感器 |
| 时钟 | PCF85063A RTC |
| 无线 | 出厂应用使用 2.4 GHz Wi-Fi |

## 📦 快速开始

1. 在 [firmware](firmware/) 下载最新的 `FactoryOnly` BIN。
2. 通过 USB 连接开发板，按[固件烧录说明](firmware/README_ZH.md)操作。
3. 将合并 BIN 烧录到 **0x0**，复位后进入桌面。

桌面操作、Wi-Fi 配网和徽章上传方法，请参阅[出厂应用使用说明](examples/factory/README_ZH.md)。

## 🧪 示例

### ESP-IDF

| 示例 | 功能 |
| --- | --- |
| [01_exio](examples/esp-idf/01_exio/README_ZH.md) | 11 路扩展 IO 控制 |
| [02_I2C_qmi8658](examples/esp-idf/02_I2C_qmi8658/README_ZH.md) | 六轴运动传感器 |
| [03_SD_Card](examples/esp-idf/03_SD_Card/README_ZH.md) | microSD 读写测试 |
| [04_I2C_pcf85063](examples/esp-idf/04_I2C_pcf85063/README_ZH.md) | RTC 时钟与网络对时 |
| [05_shtc3](examples/esp-idf/05_shtc3/README_ZH.md) | 温湿度检测 |
| [06_lvgl_demo](examples/esp-idf/06_lvgl_demo/README_ZH.md) | LVGL 显示与触摸 |
| [07_battery_voltage](examples/esp-idf/07_battery_voltage/README_ZH.md) | 电池电压采样 |

### Arduino

| 示例 | 功能 |
| --- | --- |
| [01_exio](examples/arduino/examples/01_exio/README_ZH.md) | 11 路扩展 IO 控制 |
| [02_I2C_qmi8658](examples/arduino/examples/02_I2C_qmi8658/README_ZH.md) | 六轴运动传感器 |
| [03_SD_Card](examples/arduino/examples/03_SD_Card/README_ZH.md) | microSD 读写测试 |
| [04_I2C_pcf85063](examples/arduino/examples/04_I2C_pcf85063/README_ZH.md) | RTC 时钟与网络对时 |
| [05_shtc3](examples/arduino/examples/05_shtc3/README_ZH.md) | 温湿度检测 |
| [06_lvgl_demo](examples/arduino/examples/06_lvgl_demo/README_ZH.md) | LVGL 显示与触摸 |
| [07_battery_voltage](examples/arduino/examples/07_battery_voltage/README_ZH.md) | 电池电压采样 |

## 🛠️ 开发环境

| 框架 / 库 | 版本 | 用途 |
| --- | --- | --- |
| ESP-IDF | v5.5.4 | 7 个独立示例和出厂应用 |
| Arduino-ESP32 | 3.3.12 | 7 个独立示例 |
| LVGL（IDF / Arduino） | 9.5.0 / 8.4.0 | 显示与触摸界面 |
| SensorLib（Arduino） | 0.3.1 | QMI8658 |

请先阅读 [ESP-IDF 入门指南](docs/getting-started-idf_ZH.md)或 [Arduino 入门指南](docs/getting-started-arduino_ZH.md)，再按照各示例说明运行。Arduino 配置为 **16 MB Flash、DIO、16M Flash (3MB APP/9.9MB FATFS)、USB CDC On Boot Enabled**。无需 PSRAM 配置。

## 📚 资料

- [出厂应用使用说明](examples/factory/README_ZH.md)
- [出厂固件与烧录方法](firmware/README_ZH.md)
- [板级支持与引脚定义](examples/factory/components/esp32_c6_touch_lcd_2_73/include/bsp/esp32_c6_touch_lcd_2_73.h)
- [板级 IO 与显示格式](docs/board-io_ZH.md)
- [底板原理图](hardware/schematic/ESP32-C6-Touch-LCD-2.73.pdf)

## 🤝 支持

产品使用、硬件或订单问题，请联系[微雪技术支持](https://service.waveshare.com/)。

## 📄 许可证

Apache License 2.0，详见 [LICENSE](LICENSE)；随附第三方组件保留各自的许可证声明。
