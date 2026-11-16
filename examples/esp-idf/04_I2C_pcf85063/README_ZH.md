# PCF85063 实时时钟

[English](README.md) | [简体中文](README_ZH.md)

## 功能说明

设置演示日期时间，再每秒读取一次板载 RTC。

## 硬件准备

ESP32-C6-Touch-LCD-2.73。开发板和 USB 数据线。

## 运行方法

1. 按照 [ESP-IDF 入门指南](../../../docs/getting-started-idf_ZH.md)配置环境，在 ESP-IDF v5.5.4 终端中打开本项目。
2. 执行 `idf.py build`，再执行 `idf.py flash monitor`。
3. 观察上电后的串口时间。

## 预期现象

每次启动均从 2026-10-24 12:00:00 开始走时。本例不初始化液晶，也不进行网络对时。

## 常见问题

启动时写入演示时间属于预期行为。测试断电保持时，请先删除 bsp_rtc_set_datetime 调用后重新编译，并保证 RTC 持续供电。

引脚定义请参阅[板级 IO 指南](../../../docs/board-io_ZH.md)。
