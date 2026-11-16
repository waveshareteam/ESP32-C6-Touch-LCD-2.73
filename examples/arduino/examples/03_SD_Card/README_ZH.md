# SD 卡

[English](README.md) | [简体中文](README_ZH.md)

## 功能说明

在液晶上显示 SD 卡容量、剩余空间和文件读写校验结果。

## 硬件准备

ESP32-C6-Touch-LCD-2.73。开发板、USB 数据线和 FAT32 microSD 卡。测试前请另存卡上原有的 /test.txt。

## 运行方法

1. 按照 [Arduino 入门指南](../../../../docs/getting-started-arduino_ZH.md)配置环境，打开 `03_SD_Card.ino`。
2. 选择 **ESP32C6 Dev Module** 并上传，打开 **115200 波特率**串口监视器，再按 RESET。
3. 启动前插入 SD 卡，查看屏幕结果，按 RESET 可重复测试。

## 预期现象

示例覆盖写入 /test.txt 并读回，屏幕显示容量、剩余空间、Write / read: PASS 和校验后的文本。未插卡或读写失败时显示提示，测试文件保留在卡上。

## 常见问题

示例不会格式化卡。SD 与液晶共用 SPI 引脚，因此先完成卡测试，再点亮液晶显示结果。挂载失败时请检查 FAT32 格式和插卡，再按 RESET。

引脚和板载用途请参阅[板级 IO 指南](../../../../docs/board-io_ZH.md)。
