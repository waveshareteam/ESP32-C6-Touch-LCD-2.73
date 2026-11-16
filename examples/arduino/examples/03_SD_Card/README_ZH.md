# SD 卡

[English](README.md) | [简体中文](README_ZH.md)

## 功能说明

挂载 microSD 卡，输出容量，覆盖写入 /test.txt，再读回并校验。

## 硬件准备

ESP32-C6-Touch-LCD-2.73。开发板、USB 数据线和 FAT32 microSD 卡。运行前请另存卡上原有的 /test.txt。

## 运行方法

1. 按照 [Arduino 入门指南](../../../../docs/getting-started-arduino_ZH.md)配置环境，打开 `03_SD_Card.ino`。
2. 选择 **ESP32C6 Dev Module** 并上传，打开 **115200 波特率**串口监视器，再按 RESET。
3. 上电前插入卡，查看串口结果；按 RESET 重复测试。

## 预期现象

串口输出容量、SD write/read: PASS 和测试文本。本例不初始化液晶，测试文件保留在卡上。

## 常见问题

示例不会格式化卡。挂载失败时检查插卡和 FAT32 格式；读写失败与挂载失败会分别报告。

引脚定义请参阅[板级 IO 指南](../../../../docs/board-io_ZH.md)。
