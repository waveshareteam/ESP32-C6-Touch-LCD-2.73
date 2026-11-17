# 扩展 IO

[English](README.md) | [简体中文](README_ZH.md)

## 功能说明

在触摸屏上控制全部 11 路 EXIO4–EXIO14 输出，测试过程中屏幕保持点亮。

## 硬件准备

ESP32-C6-Touch-LCD-2.73。开发板和 USB 数据线。先断开测试脚的外部负载，可选用万用表测量各脚对 GND 的电压。

## 运行方法

1. 按照 [Arduino 入门指南](../../../../docs/getting-started-arduino_ZH.md)配置环境，打开 `01_exio.ino`。
2. 选择 **ESP32C6 Dev Module** 并上传，打开 **115200 波特率**串口监视器，再按 RESET。
3. 点击 EXIO 卡片切换输出电平，点击 ALL LOW 将 11 路全部置低。

## 预期现象

启动时将 EXIO4–EXIO14 设为 LOW，写入成功后更新卡片的 LOW/HIGH 状态，失败时保留上次成功的状态。EXIO0/1 为复位脚、EXIO2 用于背光、EXIO3 保持充电检测输入。

## 常见问题

卡片显示输出命令状态，不代表电气回环测量结果。请测量 BSP 规定的 11 路排针输出；-- 或红色提示表示对应操作未成功。

引脚和板载用途请参阅[板级 IO 指南](../../../../docs/board-io_ZH.md)。
