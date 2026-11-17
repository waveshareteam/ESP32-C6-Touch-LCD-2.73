# Board IO

[English](board-io.md) | [简体中文](board-io_ZH.md)

These examples target ESP32-C6-Touch-LCD-2.73 with **16 MB Flash and no PSRAM**. See the [baseboard schematic](../hardware/schematic/ESP32-C6-Touch-LCD-2.73.pdf) for hardware connections; the example configuration targets the 16 MB hardware.

| Function | Connection |
| --- | --- |
| I²C | SDA GPIO20, SCL GPIO2, 400 kHz |
| Shared LCD/SD SPI | SCLK GPIO0, MOSI GPIO1, MISO GPIO18 |
| ILI9488 LCD | CS GPIO8, DC GPIO19; 320 × 320 |
| microSD | CS GPIO10 |
| CH32V006 IO expander | I²C address 0x24 |
| LCD / touch reset | EXIO0 / EXIO1 |
| Backlight function | EXIO2 / PWM register 0x05 |
| GT911 touch | Probe 0x5D, then 0x14; polled without an interrupt pin |
| Charging signal | EXIO3, input |
| BOOT / LEFT / RIGHT | GPIO9 / GPIO4 / GPIO5 |
| Factory PinsTest GPIO | GPIO9, 3, 15, 23, 13, 12, 22, 21 |
| Factory PinsTest EXIO | EXIO4–EXIO14 |

## IO Expander

Mode register 0x02 and output register 0x03 use **16-bit little-endian values**. The default direction mask is 0xFFF7: EXIO3 remains an input. The standalone EXIO demo provides eleven independent touchscreen controls for EXIO4–EXIO14, matching the factory BSP allowlist. These outputs start LOW; ALL LOW resets them together. EXIO0/1 retain their reset roles, EXIO2 serves the backlight, EXIO3 stays an input and EXIO15 is outside the test allowlist. The backlight stays on during testing.

Backlight PWM uses register 0x05 and is limited to **97%**, matching the factory BSP. Battery ADC register 0x06 returns a 12-bit reading (0–4095); the examples average eight samples and use `voltage = ADC × 3.3 / 4095 × 3`. This reading does not independently establish battery presence, charging state or remaining capacity.

## Display and Libraries

LVGL renders RGB565 pixels; the ILI9488 driver converts them to the LCD's RGB666 transfer format. Image/UI colors should follow the LVGL pixel configuration. Do not change pixel byte order just to match the transfer bit depth.

IDF examples contain their own copy of the factory BSP. Arduino sketches share [board_config](../examples/arduino/libraries/board_config/), with pins in [pin_config.h](../examples/arduino/libraries/board_config/src/pin_config.h). Arduino uses LVGL 8.4.0 with `LV_COLOR_DEPTH=16` and `LV_COLOR_16_SWAP=0`; IDF uses LVGL 9.5.0 with its adapter configuration. Their display integrations should not be interchanged without matching the buffer format.
