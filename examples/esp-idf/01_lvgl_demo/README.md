# LVGL Widgets

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Run the LVGL widgets demonstration on the 320 × 320 LCD with GT911 touch.

## Hardware Required

ESP32-C6-Touch-LCD-2.73. The board and a USB data cable.

## How to Run

1. Complete the [ESP-IDF first-use guide](../../../docs/getting-started-idf.md), then open this project in an ESP-IDF v5.5.4 terminal.
2. Run `idf.py build`, then `idf.py flash monitor`.
3. Tap the controls and scroll through the widget tabs.

## Expected Result

The screen lights up and widgets respond to touch. LVGL 9.5.0 uses partial draw buffers for the ESP32-C6 without PSRAM.

## Troubleshooting

If touch does not respond, reset the board and check the GT911 initialization messages. Use the supplied ILI9488 BSP and LVGL configuration.

For pin definitions, see the [board IO guide](../../../docs/board-io.md).
