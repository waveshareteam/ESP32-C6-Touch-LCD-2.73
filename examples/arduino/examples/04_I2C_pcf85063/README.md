# PCF85063 RTC

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Set a demonstration date/time, then read the onboard RTC once per second.

## Hardware Required

ESP32-C6-Touch-LCD-2.73. The board and a USB data cable.

## How to Run

1. Complete the [Arduino first-use guide](../../../../docs/getting-started-arduino.md), then open `04_I2C_pcf85063.ino`.
2. Upload to **ESP32C6 Dev Module** and open Serial Monitor at **115200 baud**. Press RESET.
3. Observe the serial timestamps after startup.

## Expected Result

The RTC starts at 2025-09-09 14:51:30 on every boot and advances once per second. The LCD is not initialized. No network time is used.

## Troubleshooting

The startup write is intentional. To test time retention, remove i2c_rtc_setTime from setup before rebuilding, and keep the RTC supply present.

For pin definitions, see the [board IO guide](../../../../docs/board-io.md).
