# SHTC3 温湿度

[English](README.md) | [简体中文](README_ZH.md)

## 功能说明

读取板载温度和相对湿度传感器。

## 硬件准备

ESP32-C6-Touch-LCD-2.73。开发板和 USB 数据线。

## 运行方法

1. 按照 [ESP-IDF 入门指南](../../../docs/getting-started-idf_ZH.md)配置环境，在 ESP-IDF v5.5.4 终端中打开本项目。
2. 执行 `idf.py build`，再执行 `idf.py flash monitor`。
3. 观察串口数据，等待板上温度稳定。

## 预期现象

每秒输出温度（°C）和相对湿度（%RH）。本例不初始化液晶。

## 常见问题

读取失败时请复位并检查 I²C 供电和连接；板上发热与气流会影响读数。

引脚定义请参阅[板级 IO 指南](../../../docs/board-io_ZH.md)。
