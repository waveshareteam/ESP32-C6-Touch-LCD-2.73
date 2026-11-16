# Battery Voltage

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Show battery voltage, the 12-bit ADC reading and sample averaging on the LCD.

## Hardware Required

ESP32-C6-Touch-LCD-2.73. The board and a USB data cable; a compatible battery for battery measurements.

## How to Run

1. Complete the [Arduino guide](../../../../docs/getting-started-arduino.md) and open `07_battery_voltage.ino`.
2. Upload to **ESP32C6 Dev Module**; open Serial Monitor at **115200 baud** and press RESET.
3. Compare the displayed voltage with a multimeter at the battery terminals.

## Expected Result

Voltage and ADC cards refresh about once per second. Each reading averages eight samples; voltage = ADC × 3.3 / 4095 × 3. Failed ADC reads show -- and an error.

## Troubleshooting

This is a nominal voltage measurement, not a fuel-gauge percentage. USB-only readings cannot establish battery presence or remaining capacity; reference voltage and divider tolerance affect accuracy.

See the [board IO guide](../../../../docs/board-io.md) for pins and reserved functions.
