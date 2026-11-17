# LVGL Display and Touch

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Show a square-screen LVGL 8.4.0 interface with touch coordinates, a touch button and a backlight slider.

## Hardware Required

ESP32-C6-Touch-LCD-2.73. The board and a USB data cable.

## How to Run

1. Complete the [Arduino guide](../../../../docs/getting-started-arduino.md) and open `06_lvgl_demo.ino`.
2. Upload to **ESP32C6 Dev Module**; open Serial Monitor at **115200 baud** and press RESET.
3. Tap the controls and move or scroll through the interface.

## Expected Result

The LCD shows the latest touch coordinates. The button responds and backlight adjusts from 5% to 97%.

## Troubleshooting

Use the supplied LVGL version and configuration. Touch uses the board GT911 driver and native 320 × 320 coordinates.

See the [board IO guide](../../../../docs/board-io.md) for pins and reserved functions.
