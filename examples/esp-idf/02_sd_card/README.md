# SD Card

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Mount a microSD card, overwrite /test.txt, then read and compare its contents. Display the result on the LCD.

## Hardware Required

ESP32-C6-Touch-LCD-2.73. The board, a USB data cable and a FAT32 microSD card. Copy any existing /test.txt elsewhere before running.

## How to Run

1. Complete the [ESP-IDF first-use guide](../../../docs/getting-started-idf.md), then open this project in an ESP-IDF v5.5.4 terminal.
2. Run `idf.py build`, then `idf.py flash monitor`.
3. Insert the card before startup; read the LCD result and serial output. Press RESET to repeat.

## Expected Result

The LCD reports Write / read passed with the test text. The card is unmounted after the test; /test.txt remains on it. A missing card produces an LCD hint.

## Troubleshooting

The example does not format cards. If mounting fails, check insertion and FAT32 formatting. A failed write/read is reported separately from a mount failure.

For pin definitions, see the [board IO guide](../../../docs/board-io.md).
