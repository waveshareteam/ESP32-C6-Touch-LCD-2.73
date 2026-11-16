<div align="center">
  <h1>ESP32-C6-Touch-LCD-2.73</h1>
  <p><strong>搭载 2.73 英寸 320 × 320 触摸液晶屏的 ESP32-C6 开发板</strong></p>
  <p><a href="README.md">English</a> | <strong>简体中文</strong></p>
  <p><a href="firmware/">📦 出厂固件</a> &middot; <a href="examples/factory/">🧩 出厂源码</a></p>
</div>

---

## ✨ 概述

本仓库提供 ESP32-C6-Touch-LCD-2.73 的出厂演示源码与可直接烧录的合并固件。触摸桌面包含显示、触摸、传感器和 GPIO 测试，以及电子时钟、电子徽章和游戏。

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

## 🧪 示例

| 工程 | 主要功能 |
| --- | --- |
| [出厂应用](examples/factory/README_ZH.md) | 11 个 APP：Display Test、Settings、Touch Test、Ball、Weather、Sensor、2048、Button Test、PinsTest、Badge 和 Watch。 |

电子徽章将图片保存在板载 Flash，支持手机上传。本版本不包含 SD 卡相册 Photos；操作方式和配网方法见[出厂应用说明](examples/factory/README_ZH.md)。

## 🛠️ 开发环境

| 开发框架 | 版本 | 工程数 |
| --- | --- | ---: |
| ESP-IDF | v5.5.4 | 1 个出厂应用 |
| LVGL | 9.5.0 | GUI 依赖 |

在 ESP-IDF 终端进入 [examples/factory](examples/factory/README_ZH.md)，运行 `idf.py build`。ESP-IDF 组件管理器会自动下载依赖。

## 📚 资料

- [出厂应用使用说明](examples/factory/README_ZH.md)
- [出厂固件与烧录方法](firmware/README_ZH.md)
- [板级支持与引脚定义](examples/factory/components/esp32_c6_touch_lcd_2_73/include/bsp/esp32_c6_touch_lcd_2_73.h)

## 🤝 支持

产品使用、硬件或订单问题，请联系[微雪技术支持](https://service.waveshare.com/)。

## 📄 许可证

Apache License 2.0，详见 [LICENSE](LICENSE)；随附第三方组件保留各自的许可证声明。
