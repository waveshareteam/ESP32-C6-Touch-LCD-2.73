# QMI8658 IMU

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Read three-axis acceleration and angular velocity from the onboard QMI8658.

## Hardware Required

ESP32-C6-Touch-LCD-2.73. The board and a USB data cable.

## How to Run

1. Complete the [Arduino first-use guide](../../../../docs/getting-started-arduino.md), then open `02_I2C_qmi8658.ino`.
2. Upload to **ESP32C6 Dev Module** and open Serial Monitor at **115200 baud**. Press RESET.
3. Rotate and tilt the board while watching the serial monitor.

## Expected Result

Serial readings update about once per second: acceleration in m/s² and angular velocity in rad/s. The LCD is not initialized.

## Troubleshooting

If initialization fails, reset and check the I²C connection. At rest, gravity contributes approximately 9.807 m/s² across the acceleration axes.

For pin definitions, see the [board IO guide](../../../../docs/board-io.md).
