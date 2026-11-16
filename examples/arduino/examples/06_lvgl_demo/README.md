# LVGL Display and Touch

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Show a square-screen LVGL interface with touch coordinates, a touch button and a backlight slider.

## Hardware Required

ESP32-C6-Touch-LCD-2.73. The board and a USB data cable.

## How to Run

1. Complete the [Arduino first-use guide](../../../../docs/getting-started-arduino.md), then open `06_lvgl_demo.ino`.
2. Upload to **ESP32C6 Dev Module** and open Serial Monitor at **115200 baud**. Press RESET.
3. Touch different positions, press the button and move the backlight slider.

## Expected Result

The 320 × 320 screen shows the latest touch coordinates. The button responds and backlight changes between 5% and 97%. LVGL 8.4.0 uses an RGB565 partial buffer; the ILI9488 driver converts pixels to RGB666.

## Troubleshooting

If touch does not respond, reset the board and check the GT911 initialization messages. Use the supplied ILI9488 BSP and LVGL configuration.

For pin definitions, see the [board IO guide](../../../../docs/board-io.md).
