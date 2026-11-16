# SD 卡

[English](README.md) | [简体中文](README_ZH.md)

## 功能说明

挂载 microSD 卡，覆盖写入 /test.txt，再读取并核对内容，在液晶上显示结果。

## 硬件准备

ESP32-C6-Touch-LCD-2.73。开发板、USB 数据线和 FAT32 microSD 卡。运行前请另存卡上原有的 /test.txt。

## 运行方法

1. 按照 [ESP-IDF 入门指南](../../../docs/getting-started-idf_ZH.md)配置环境，在 ESP-IDF v5.5.4 终端中打开本项目。
2. 执行 `idf.py build`，再执行 `idf.py flash monitor`。
3. 上电前插入卡，查看屏幕和串口结果；按 RESET 可重复测试。

## 预期现象

液晶显示 Write / read passed 和测试文本。测试后卸载卡，保留 /test.txt；未插卡时屏幕显示提示。

## 常见问题

示例不会格式化卡。挂载失败时检查插卡和 FAT32 格式；读写失败与挂载失败会分别报告。

引脚定义请参阅[板级 IO 指南](../../../docs/board-io_ZH.md)。
