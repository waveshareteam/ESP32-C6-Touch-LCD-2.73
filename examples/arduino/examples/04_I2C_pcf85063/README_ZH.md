# PCF85063 实时时钟

[English](README.md) | [简体中文](README_ZH.md)

## 功能说明

设置演示日期时间，再每秒读取一次板载 RTC。

## 硬件准备

ESP32-C6-Touch-LCD-2.73。开发板和 USB 数据线。

## 运行方法

1. 按照 [Arduino 入门指南](../../../../docs/getting-started-arduino_ZH.md)配置环境，打开 `04_I2C_pcf85063.ino`。
2. 选择 **ESP32C6 Dev Module** 并上传，打开 **115200 波特率**串口监视器，再按 RESET。
3. 观察上电后的串口时间。

## 预期现象

每次启动均从 2025-09-09 14:51:30 开始走时。本例不初始化液晶，也不使用网络对时。

## 常见问题

启动时写入演示时间属于预期行为。测试断电保持时，先删除 setup 中的 i2c_rtc_setTime 调用后重新编译，并保证 RTC 持续供电。

引脚定义请参阅[板级 IO 指南](../../../../docs/board-io_ZH.md)。
