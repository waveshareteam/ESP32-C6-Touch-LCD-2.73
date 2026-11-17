# 温湿度传感器

[English](README.md) | [简体中文](README_ZH.md)

## 功能说明

通过两张数据卡片实时显示 SHTC3 温度和相对湿度。

## 硬件准备

ESP32-C6-Touch-LCD-2.73。开发板和 USB 数据线。

## 运行方法

1. 按照 [ESP-IDF 入门指南](../../../docs/getting-started-idf_ZH.md)配置环境，在 ESP-IDF v5.5.4 终端打开本项目。
2. 执行 `idf.py build`，再执行 `idf.py flash monitor`。
3. 观察温湿度数值，等待板上温度稳定。

## 预期现象

温度（°C）和湿度（%RH）每秒刷新一次，并通过串口输出。读取或 CRC 校验失败时显示 -- 和错误，屏幕保持运行。

## 常见问题

板上发热和气流会影响读数。错误提示表示传感器通信或 CRC 校验失败。

引脚和板载用途请参阅[板级 IO 指南](../../../docs/board-io_ZH.md)。
