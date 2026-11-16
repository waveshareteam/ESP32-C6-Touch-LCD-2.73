# PCF85063 RTC

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Set a demonstration date/time, then read the onboard RTC once per second.

## Hardware Required

ESP32-C6-Touch-LCD-2.73. The board and a USB data cable.

## How to Run

1. Complete the [ESP-IDF first-use guide](../../../docs/getting-started-idf.md), then open this project in an ESP-IDF v5.5.4 terminal.
2. Run `idf.py build`, then `idf.py flash monitor`.
3. Observe the serial timestamps after startup.

## Expected Result

The RTC starts at 2026-10-24 12:00:00 on every boot and advances once per second. The LCD is not initialized. This standalone example does not use network time.

## Troubleshooting

The startup write is intentional. To test time retention, remove the bsp_rtc_set_datetime call before rebuilding, and keep the RTC supply present.

For pin definitions, see the [board IO guide](../../../docs/board-io.md).
