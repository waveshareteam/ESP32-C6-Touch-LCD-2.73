# Battery Voltage

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Display the battery ADC reading and calculated voltage on the LCD and serial monitor.

## Hardware Required

ESP32-C6-Touch-LCD-2.73. The board and a USB data cable; a compatible battery for a meaningful battery measurement.

## How to Run

1. Complete the [ESP-IDF first-use guide](../../../docs/getting-started-idf.md), then open this project in an ESP-IDF v5.5.4 terminal.
2. Run `idf.py build`, then `idf.py flash monitor`.
3. Observe the voltage and ADC value, and compare voltage against a multimeter at the battery terminals.

## Expected Result

Readings refresh about once per second. The 12-bit ADC range is 0–4095; voltage = ADC × 3.3 / 4095 × 3. Eight samples are averaged per reading.

## Troubleshooting

This is a voltage test, not a fuel gauge or calibrated percentage. USB-only readings cannot confirm battery presence or charge level. The 3.3 V reference and divider tolerance affect accuracy.

For pin definitions, see the [board IO guide](../../../docs/board-io.md).
