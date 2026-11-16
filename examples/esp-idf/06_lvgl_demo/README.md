# LVGL Display and Touch

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Run the LVGL 9.5.0 widgets demonstration on the LCD with GT911 touch.

## Hardware Required

ESP32-C6-Touch-LCD-2.73. The board and a USB data cable.

## How to Run

1. Complete the [ESP-IDF guide](../../../docs/getting-started-idf.md) and open this project in an ESP-IDF v5.5.4 terminal.
2. Run `idf.py build`, then `idf.py flash monitor`.
3. Tap the controls and move or scroll through the interface.

## Expected Result

The LCD lights up and widget tabs, buttons and sliders respond to touch. Partial draw buffers support the ESP32-C6 without PSRAM.

## Troubleshooting

Use the supplied LVGL version and configuration. Touch uses the board GT911 driver and native 320 × 320 coordinates.

See the [board IO guide](../../../docs/board-io.md) for pins and reserved functions.
