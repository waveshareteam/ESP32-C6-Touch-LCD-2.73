# QMI8658 惯性传感器

[English](README.md) | [简体中文](README_ZH.md)

## 功能说明

读取板载 QMI8658 的三轴加速度和角速度。

## 硬件准备

ESP32-C6-Touch-LCD-2.73。开发板和 USB 数据线。

## 运行方法

1. 按照 [ESP-IDF 入门指南](../../../docs/getting-started-idf_ZH.md)配置环境，在 ESP-IDF v5.5.4 终端中打开本项目。
2. 执行 `idf.py build`，再执行 `idf.py flash monitor`。
3. 转动或倾斜开发板，观察串口数值。

## 预期现象

每 500 ms 输出一次加速度（g）和角速度（°/s）。本例不初始化液晶。

## 常见问题

初始化失败时请复位并检查 I²C 连接。静止时，加速度三轴合成值约为 1 g，包含重力分量。

引脚定义请参阅[板级 IO 指南](../../../docs/board-io_ZH.md)。
