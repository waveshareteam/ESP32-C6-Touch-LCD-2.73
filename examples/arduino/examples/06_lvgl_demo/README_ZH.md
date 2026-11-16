# LVGL 显示与触摸

[English](README.md) | [简体中文](README_ZH.md)

## 功能说明

显示适配方屏的 LVGL 界面，包含触摸坐标、触摸按钮和背光滑条。

## 硬件准备

ESP32-C6-Touch-LCD-2.73。开发板和 USB 数据线。

## 运行方法

1. 按照 [Arduino 入门指南](../../../../docs/getting-started-arduino_ZH.md)配置环境，打开 `06_lvgl_demo.ino`。
2. 选择 **ESP32C6 Dev Module** 并上传，打开 **115200 波特率**串口监视器，再按 RESET。
3. 点击不同位置，按下按钮并拖动背光滑条。

## 预期现象

320 × 320 屏幕显示最近的触摸坐标，按钮可响应，背光范围为 5%–97%。LVGL 8.4.0 使用 RGB565 局部缓冲区，由 ILI9488 驱动转换为 RGB666。

## 常见问题

触摸无响应时，请复位并查看 GT911 初始化日志。请使用随示例提供的 ILI9488 BSP 和 LVGL 配置。

引脚定义请参阅[板级 IO 指南](../../../../docs/board-io_ZH.md)。
