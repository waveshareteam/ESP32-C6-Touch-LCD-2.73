# 惯性传感器

[English](README.md) | [简体中文](README_ZH.md)

## 功能说明

在液晶上显示 QMI8658 的六轴实时数据，同时通过串口输出。

## 硬件准备

ESP32-C6-Touch-LCD-2.73。开发板和 USB 数据线。

## 运行方法

1. 按照 [Arduino 入门指南](../../../../docs/getting-started-arduino_ZH.md)配置环境，打开 `02_I2C_qmi8658.ino`。
2. 选择 **ESP32C6 Dev Module** 并上传，打开 **115200 波特率**串口监视器，再按 RESET。
3. 倾斜或转动开发板，观察加速度和角速度卡片。

## 预期现象

三轴加速度（g）和角速度（deg/s）约每 200 ms 刷新一次，读取失败时显示 -- 和错误提示，界面继续运行。

## 常见问题

静止时加速度合成值约为 1 g，包含重力分量。Arduino 示例使用同一条 Wire 总线访问传感器和 GT911 触摸。

引脚和板载用途请参阅[板级 IO 指南](../../../../docs/board-io_ZH.md)。
