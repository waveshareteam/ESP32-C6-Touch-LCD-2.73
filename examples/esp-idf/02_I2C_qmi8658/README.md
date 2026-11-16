# QMI8658 IMU

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Read three-axis acceleration and angular velocity from the onboard QMI8658.

## Hardware Required

ESP32-C6-Touch-LCD-2.73. The board and a USB data cable.

## How to Run

1. Complete the [ESP-IDF first-use guide](../../../docs/getting-started-idf.md), then open this project in an ESP-IDF v5.5.4 terminal.
2. Run `idf.py build`, then `idf.py flash monitor`.
3. Rotate and tilt the board while watching the serial monitor.

## Expected Result

Serial readings update every 500 ms: acceleration in g and angular velocity in degrees per second. The example does not initialize the LCD.

## Troubleshooting

If initialization fails, reset and check the I²C connection. At rest, gravity contributes approximately 1 g across the acceleration axes.

For pin definitions, see the [board IO guide](../../../docs/board-io.md).
