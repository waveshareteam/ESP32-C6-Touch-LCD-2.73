# LVGL 控件

[English](README.md) | [简体中文](README_ZH.md)

## 功能说明

在 320 × 320 液晶上运行 LVGL 控件演示，并使用 GT911 触摸操作。

## 硬件准备

ESP32-C6-Touch-LCD-2.73。开发板和 USB 数据线。

## 运行方法

1. 按照 [ESP-IDF 入门指南](../../../docs/getting-started-idf_ZH.md)配置环境，在 ESP-IDF v5.5.4 终端中打开本项目。
2. 执行 `idf.py build`，再执行 `idf.py flash monitor`。
3. 点击控件并滑动切换控件页。

## 预期现象

屏幕点亮，控件能够响应触摸。使用 LVGL 9.5.0 和局部绘制缓冲区，适用于无 PSRAM 的 ESP32-C6。

## 常见问题

触摸无响应时，请复位并查看 GT911 初始化日志。请使用随示例提供的 ILI9488 BSP 和 LVGL 配置。

引脚定义请参阅[板级 IO 指南](../../../docs/board-io_ZH.md)。
