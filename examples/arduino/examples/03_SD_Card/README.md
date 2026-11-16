# SD Card

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Mount a microSD card, print its capacity, overwrite /test.txt, then read and verify its contents.

## Hardware Required

ESP32-C6-Touch-LCD-2.73. The board, a USB data cable and a FAT32 microSD card. Copy any existing /test.txt elsewhere before running.

## How to Run

1. Complete the [Arduino first-use guide](../../../../docs/getting-started-arduino.md), then open `03_SD_Card.ino`.
2. Upload to **ESP32C6 Dev Module** and open Serial Monitor at **115200 baud**. Press RESET.
3. Insert the card before startup and observe the serial output. Press RESET to repeat.

## Expected Result

The serial monitor prints capacity, SD write/read: PASS and the test text. The LCD is not initialized. The test file remains on the card.

## Troubleshooting

The example does not format cards. If mounting fails, check insertion and FAT32 formatting. A failed write/read is reported separately from a mount failure.

For pin definitions, see the [board IO guide](../../../../docs/board-io.md).
