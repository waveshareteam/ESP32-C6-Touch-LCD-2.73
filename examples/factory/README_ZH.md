# 出厂应用

[English](README.md) | [简体中文](README_ZH.md)

## 功能

320 × 320 桌面包含 11 个 APP：Display Test、Settings、Touch Test、Ball、Weather、Sensor、2048、Button Test、PinsTest、Badge 和 Watch。电子徽章可在 Flash 保存 6 张图片，本版本不包含 SD 卡相册 Photos。

## 硬件需求

ESP32-C6-Touch-LCD-2.73 开发板及 USB 数据线。microSD 卡为可选项，可在 Settings 中查看容量；应用不需要音频硬件或 PSRAM。

## 如何运行

使用 ESP-IDF v5.5.4，在当前目录执行以下命令，组件管理器会自动下载依赖。

```sh
idf.py set-target esp32c6
idf.py build
idf.py flash monitor
```

## 预期现象

桌面每页 4 个图标，左右滑动翻页，多数 APP 从底部中间向上滑动返回。Display Test 双击退出；Touch Test 点击 UI 隐藏工具栏，按钮变为 Exit，再点击返回桌面。

进入 Settings → Wi-Fi setup 或 Badge → Add/Manage images，使用密码 `12345678` 连接设备热点，再打开 **http://192.168.4.1** 配网或上传徽章图片。Watch 与桌面开机读取有效 RTC 日历；联网校时后同步系统时钟及 RTC，断网后继续走时。

## 常见问题

仅支持 2.4 GHz Wi-Fi；没有已保存的网络时，程序尝试连接 `weather-wifi`，密码为 `12345678`，其他网络可在 Settings 中选择。电子徽章不需要互联网或 microSD 卡，未插卡会在 Settings 中显示提示。

PinsTest 未操作时显示 `--`，首次点击输出高电平，之后点击交替切换高低电平。测试 IO 为 GPIO9/3/15/23/13/12/22/21 与 EXIO4～14，退出时恢复 MCU 测试引脚为输入。
