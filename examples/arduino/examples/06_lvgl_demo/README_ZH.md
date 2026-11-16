# LVGL 显示与触摸

[English](README.md) | [简体中文](README_ZH.md)

## 功能说明

显示适配方屏的 LVGL 8.4.0 界面，包含触摸坐标、触摸按钮和背光滑条。

## 硬件准备

ESP32-C6-Touch-LCD-2.73。开发板和 USB 数据线。

## 运行方法

1. 按照 [Arduino 入门指南](../../../../docs/getting-started-arduino_ZH.md)配置环境，打开 `06_lvgl_demo.ino`。
2. 选择 **ESP32C6 Dev Module** 并上传，打开 **115200 波特率**串口监视器，再按 RESET。
3. 点击控件，并拖动或滑动操作界面。

## 预期现象

屏幕显示最近的触摸坐标，按钮响应触摸，背光可在 5%–97% 间调节。

## 常见问题

请使用随示例提供的 LVGL 版本和配置。触摸使用板载 GT911 驱动及 320 × 320 原始坐标。

引脚和板载用途请参阅[板级 IO 指南](../../../../docs/board-io_ZH.md)。
