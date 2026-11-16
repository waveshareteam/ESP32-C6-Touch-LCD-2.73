# SHTC3 温湿度

[English](README.md) | [简体中文](README_ZH.md)

## 功能说明

读取板载温度和相对湿度传感器。

## 硬件准备

ESP32-C6-Touch-LCD-2.73。开发板和 USB 数据线。

## 运行方法

1. 按照 [Arduino 入门指南](../../../../docs/getting-started-arduino_ZH.md)配置环境，打开 `05_shtc3.ino`。
2. 选择 **ESP32C6 Dev Module** 并上传，打开 **115200 波特率**串口监视器，再按 RESET。
3. 观察串口数据，等待板上温度稳定。

## 预期现象

每秒输出温度（°C）和相对湿度（%RH）。本例不初始化液晶。

## 常见问题

读取失败时请复位并检查 I²C 供电和连接；板上发热与气流会影响读数。

引脚定义请参阅[板级 IO 指南](../../../../docs/board-io_ZH.md)。
