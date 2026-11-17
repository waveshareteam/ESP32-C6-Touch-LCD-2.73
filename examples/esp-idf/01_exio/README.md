# IO Expander

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Control all eleven exposed EXIO4–EXIO14 outputs from the touchscreen. The backlight stays on.

## Hardware Required

ESP32-C6-Touch-LCD-2.73. The board and a USB data cable. Disconnect external loads from the test pins; optionally measure them against GND with a multimeter.

## How to Run

1. Complete the [ESP-IDF guide](../../../docs/getting-started-idf.md) and open this project in an ESP-IDF v5.5.4 terminal.
2. Run `idf.py build`, then `idf.py flash monitor`.
3. Tap any EXIO tile to toggle its commanded output. Tap ALL LOW to drive all eleven low.

## Expected Result

EXIO4–EXIO14 are driven LOW at startup. Successful writes update each tile to LOW/HIGH; failed writes retain the last successful state. EXIO0/1 are reset pins, EXIO2 serves the backlight and EXIO3 remains a charge input.

## Troubleshooting

The tiles show commanded output levels, not an electrical loopback measurement. Use the eleven header outputs listed in the BSP; a -- or red message means the corresponding operation failed.

See the [board IO guide](../../../docs/board-io.md) for pins and reserved functions.
