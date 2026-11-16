# SHTC3 Temperature and Humidity

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Read the onboard temperature and relative-humidity sensor.

## Hardware Required

ESP32-C6-Touch-LCD-2.73. The board and a USB data cable.

## How to Run

1. Complete the [ESP-IDF first-use guide](../../../docs/getting-started-idf.md), then open this project in an ESP-IDF v5.5.4 terminal.
2. Run `idf.py build`, then `idf.py flash monitor`.
3. Observe the serial readings; allow the board temperature to stabilize.

## Expected Result

Temperature in °C and relative humidity in %RH are printed once per second. The LCD is not initialized.

## Troubleshooting

If reads fail, reset and check I²C power and connections. Board heating and airflow can affect readings.

For pin definitions, see the [board IO guide](../../../docs/board-io.md).
