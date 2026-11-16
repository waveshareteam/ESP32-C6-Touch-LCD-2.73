# 板级 IO

[English](board-io.md) | [简体中文](board-io_ZH.md)

这些示例适用于 **16 MB Flash、无 PSRAM** 的 ESP32-C6-Touch-LCD-2.73。附带的[底板原理图](../hardware/schematic/ESP32-C6-LCD-2.73.pdf)仍带有旧版 N8 模组标注，示例配置对应 16 MB 实物。

| 功能 | 连接 |
| --- | --- |
| I²C | SDA GPIO20、SCL GPIO2，400 kHz |
| LCD/SD 共用 SPI | SCLK GPIO0、MOSI GPIO1、MISO GPIO18 |
| ILI9488 液晶 | CS GPIO8、DC GPIO19；320 × 320 |
| microSD | CS GPIO10 |
| CH32V006 扩展 IO | I²C 地址 0x24 |
| 液晶 / 触摸复位 | EXIO0 / EXIO1 |
| 背光功能 | EXIO2 / PWM 寄存器 0x05 |
| GT911 触摸 | 依次探测 0x5D、0x14，不使用中断脚，轮询读取 |
| 充电信号 | EXIO3，输入 |
| BOOT / LEFT / RIGHT | GPIO9 / GPIO4 / GPIO5 |
| 出厂 PinsTest GPIO | GPIO9、3、15、23、13、12、22、21 |
| 出厂 PinsTest EXIO | EXIO4–EXIO14 |

## 扩展 IO

模式寄存器 0x02 和输出寄存器 0x03 使用 **16 位小端数据**。默认方向掩码为 0xFFF7，EXIO3 保持输入。独立 EXIO 示例按照出厂 BSP 的允许范围，提供 EXIO4–EXIO14 共 11 路独立触摸控制。启动时输出 LOW，ALL LOW 可全部置低。EXIO0/1 保持复位用途、EXIO2 用于背光、EXIO3 保持输入，EXIO15 不在测试允许范围内。测试过程中背光保持点亮。

背光使用寄存器 0x05，上限为 **97%**，与出厂 BSP 一致。电池 ADC 寄存器 0x06 返回 12 位读数（0–4095），示例平均 8 个样本，按 `电压 = ADC × 3.3 / 4095 × 3` 换算。该读数不能独立判断电池是否接入、充电状态或剩余容量。

## 显示与依赖库

LVGL 使用 RGB565 绘制像素，ILI9488 驱动再转换为液晶传输所需的 RGB666。图片和 UI 颜色应遵循 LVGL 像素配置，不能仅因传输位深不同就调整字节序。

IDF 示例各自包含出厂 BSP 副本。Arduino 示例共用 [board_config](../examples/arduino/libraries/board_config/)，引脚集中在 [pin_config.h](../examples/arduino/libraries/board_config/src/pin_config.h)。Arduino 使用 LVGL 8.4.0，配置为 `LV_COLOR_DEPTH=16`、`LV_COLOR_16_SWAP=0`；IDF 使用 LVGL 9.5.0 及其适配层配置。两者的显示集成需与缓冲格式对应，不宜直接互换。
