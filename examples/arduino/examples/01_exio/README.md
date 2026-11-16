# IO Expander

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Toggle the EXIO6/EXIO7 header outputs and backlight through the CH32V006.

## Hardware Required

ESP32-C6-Touch-LCD-2.73. The board and a USB data cable; optionally a multimeter between EXIO6/7 and GND. Disconnect external loads from these outputs first.

## How to Run

1. Complete the [Arduino first-use guide](../../../../docs/getting-started-arduino.md), then open `01_exio.ino`.
2. Upload to **ESP32C6 Dev Module** and open Serial Monitor at **115200 baud**. Press RESET.
3. Measure the EXIO6/7 outputs and observe the backlight.

## Expected Result

Both outputs alternate low/high every second. Backlight PWM alternates 0%/97%. No LCD image is initialized. EXIO3 remains an input.

## Troubleshooting

A blank image is expected: this is an IO/PWM test. Use EXIO6/7, not the display/touch reset pins. See the board IO guide.

For pin definitions, see the [board IO guide](../../../../docs/board-io.md).
