# ESP32-C6-Touch-LCD-2.73 BSP

Use `bsp_init()`, create the UI under `bsp_display_lock()`, then call `bsp_display_show()`.

- `esp32_c6_touch_lcd_2_73.c`: shared I2C/SPI, ILI9488 320x320 RGB666 output, GT911 and LVGL adapter.
- `bsp_board.c`: startup status, BOOT/LEFT/RIGHT, PCF85063A, QMI8658, SHTC3, battery ADC, SD leases and 2.4 GHz Wi-Fi.
- `include/bsp/board.h`: shared application-facing API.
- `io_extension`: checked and serialized CH32V006 access using the original 16-bit output and 12-bit ADC protocol.

I2C SDA/SCL: GPIO20/2. LCD MOSI/MISO/SCLK/DC/CS: GPIO1/18/0/19/8. SD CS: GPIO10. LCD reset: EXIO0; GT911 reset: EXIO1; charge detect: EXIO3 input. BOOT/LEFT/RIGHT: GPIO9/4/5, active low.

Exposed MCU test outputs: GPIO9/3/15/23/13/12/22/21. Exposed expander test outputs: EXIO4..14. PinsTest restores MCU test pins to input on exit.

Display buffers are internal RAM only. RTC and system time remain independent. SD leases prevent unmount while a client owns files; unmount never frees the SPI bus shared with the LCD. The battery API returns voltage, not a verified state-of-charge percentage.
