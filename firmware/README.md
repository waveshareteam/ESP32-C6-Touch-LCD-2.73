# Factory Firmware

[English](README.md) | [简体中文](README_ZH.md)

Each BIN is a merged image for **ESP32-C6-Touch-LCD-2.73**, flashed at **0x0**. The date in its filename identifies the version; the newest version is listed first.

| File | Purpose |
| --- | --- |
| [ESP32-C6-Touch-LCD-2.73_FactoryOnly_20261116.bin](ESP32-C6-Touch-LCD-2.73_FactoryOnly_20261116.bin) | Eleven-app factory desktop, including Badge, Watch and PinsTest. SD-card Photos gallery removed. |

Download the BIN, install esptool, connect the board, and replace `PORT` below with its serial port:

```sh
python -m pip install esptool
python -m esptool --chip esp32c6 --port PORT write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB 0x0 ESP32-C6-Touch-LCD-2.73_FactoryOnly_20261116.bin
```

In Flash Download Tool, select ESP32-C6, add this single BIN at `0x0`, and use DIO, 80 MHz and 16 MB. Reset the board after flashing.

Factory source and app controls: [examples/factory](../examples/factory/README.md).
