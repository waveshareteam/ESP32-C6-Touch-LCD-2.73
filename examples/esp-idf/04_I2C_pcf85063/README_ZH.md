# 实时时钟

[English](README.md) | [简体中文](README_ZH.md)

## 功能说明

在液晶上显示 PCF85063 硬件时间、日期和星期。连接 Wi-Fi 后通过 SNTP 同步北京时间（UTC+8），并写入硬件 RTC。

## 硬件准备

ESP32-C6-Touch-LCD-2.73、USB 数据线，以及可访问互联网的 2.4 GHz Wi-Fi 网络，用于网络对时。

## 运行方法

1. 按照 [ESP-IDF 入门指南](../../../docs/getting-started-idf_ZH.md)配置环境，在 ESP-IDF v5.5.4 终端打开本项目。
2. 在 [`wifi_config.h`](main/wifi_config.h) 中填写 `RTC_WIFI_SSID`（网络名称）和 `RTC_WIFI_PASSWORD`（密码）。
3. 执行 `idf.py build`，再执行 `idf.py flash monitor`。

## 预期现象

启动时读取已有 RTC 时间，不重新设置固定日期。收到网络时间后，屏幕显示北京时间并将其写入 RTC；屏幕每秒刷新，断网后继续读取 RTC 计时。

## 常见问题

SSID 留空时只读取 RTC；一直等待 NTP 时请检查网络是否可以访问互联网。RTC 日期无效时，BSP 先设置为 2025-01-01 12:00:00，之后等待网络对时；RTC 通信异常时显示 `RTC unavailable`。掉电保持时间需要维持 RTC 供电。

引脚和板载用途请参阅[板级 IO 指南](../../../docs/board-io_ZH.md)。
