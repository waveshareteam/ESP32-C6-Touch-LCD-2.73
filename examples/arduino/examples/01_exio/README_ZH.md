# 扩展 IO

[English](README.md) | [简体中文](README_ZH.md)

## 功能说明

通过 CH32V006 切换排针 EXIO6/EXIO7 输出和背光。

## 硬件准备

ESP32-C6-Touch-LCD-2.73。开发板和 USB 数据线，可选用万用表测量 EXIO6/7 对 GND 的电压。请先断开这些引脚的外部负载。

## 运行方法

1. 按照 [Arduino 入门指南](../../../../docs/getting-started-arduino_ZH.md)配置环境，打开 `01_exio.ino`。
2. 选择 **ESP32C6 Dev Module** 并上传，打开 **115200 波特率**串口监视器，再按 RESET。
3. 测量 EXIO6/7 电平并观察背光变化。

## 预期现象

两个输出每秒在低/高电平间切换，背光 PWM 在 0%/97% 间切换。不初始化液晶画面，EXIO3 保持输入。

## 常见问题

本例仅测试 IO/PWM，无显示内容属于正常现象。测量 EXIO6/7，请勿使用屏幕或触摸的复位脚。引脚说明见板级 IO 指南。

引脚定义请参阅[板级 IO 指南](../../../../docs/board-io_ZH.md)。
