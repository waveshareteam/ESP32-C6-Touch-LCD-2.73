# 出厂固件

[English](README.md) | [简体中文](README_ZH.md)

这里的 BIN 均为 **ESP32-C6-Touch-LCD-2.73** 合并固件，烧录地址为 **0x0**。文件名中的日期用于区分版本，最新版本排在前面。

| 文件 | 用途 |
| --- | --- |
| [ESP32-C6-Touch-LCD-2.73_FactoryOnly_20261116.bin](ESP32-C6-Touch-LCD-2.73_FactoryOnly_20261116.bin) | 11 个 APP 的出厂桌面，包含电子徽章、电子时钟及 PinsTest，已移除 SD 卡相册 Photos。 |

下载 BIN、安装 esptool 并连接开发板，将下方的 `PORT` 替换为设备串口后执行：

```sh
python -m pip install esptool
python -m esptool --chip esp32c6 --port PORT write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB 0x0 ESP32-C6-Touch-LCD-2.73_FactoryOnly_20261116.bin
```

使用 Flash Download Tool 时选择 ESP32-C6，仅添加这个 BIN，地址填 `0x0`，参数选择 DIO、80 MHz、16 MB。烧录后复位开发板。

出厂源码及 APP 操作说明：[examples/factory](../examples/factory/README_ZH.md)。
