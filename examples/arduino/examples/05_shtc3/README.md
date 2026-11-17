# Environment Sensor

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Show SHTC3 temperature and relative humidity in two live LCD cards.

## Hardware Required

ESP32-C6-Touch-LCD-2.73. The board and a USB data cable.

## How to Run

1. Complete the [Arduino guide](../../../../docs/getting-started-arduino.md) and open `05_shtc3.ino`.
2. Upload to **ESP32C6 Dev Module**; open Serial Monitor at **115200 baud** and press RESET.
3. Watch the temperature/humidity values and allow the board temperature to stabilize.

## Expected Result

Temperature in degrees C and humidity in %RH refresh once per second and are also printed to serial. Read/CRC failures show -- and an error while the screen continues running.

## Troubleshooting

Board heating and airflow affect measurements. An error indicates a failed sensor transaction or CRC check, not a failed LCD.

See the [board IO guide](../../../../docs/board-io.md) for pins and reserved functions.
