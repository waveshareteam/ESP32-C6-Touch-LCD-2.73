# Motion Sensor

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Show six live QMI8658 measurements on the LCD and print them to serial.

## Hardware Required

ESP32-C6-Touch-LCD-2.73. The board and a USB data cable.

## How to Run

1. Complete the [ESP-IDF guide](../../../docs/getting-started-idf.md) and open this project in an ESP-IDF v5.5.4 terminal.
2. Run `idf.py build`, then `idf.py flash monitor`.
3. Tilt and rotate the board while watching the acceleration and angular-velocity cards.

## Expected Result

Three-axis acceleration in g and angular velocity in deg/s refresh about every 200 ms. A read error clears the values to -- and shows an error without stopping the display.

## Troubleshooting

At rest, the combined acceleration is about 1 g because of gravity. On Arduino, the sensor shares the existing Wire bus with the GT911 touch controller.

See the [board IO guide](../../../docs/board-io.md) for pins and reserved functions.
