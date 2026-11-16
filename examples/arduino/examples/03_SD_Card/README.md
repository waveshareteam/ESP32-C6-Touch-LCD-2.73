# SD Card

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Show SD capacity, free space and a file write/read verification result on the LCD.

## Hardware Required

ESP32-C6-Touch-LCD-2.73. The board, a USB data cable and a FAT32 microSD card. Preserve any existing /test.txt before testing.

## How to Run

1. Complete the [Arduino guide](../../../../docs/getting-started-arduino.md) and open `03_SD_Card.ino`.
2. Upload to **ESP32C6 Dev Module**; open Serial Monitor at **115200 baud** and press RESET.
3. Insert the card before startup. Observe the LCD result; press RESET to repeat.

## Expected Result

The example overwrites /test.txt, reads it back, then shows capacity/free space and Write / read: PASS with verified text. Missing-card and read/write errors are shown on screen. The file remains on the card.

## Troubleshooting

The example never formats a card. SD operations finish before LCD transfers begin on their shared SPI pins; the result screen appears after the test. Insert a FAT32 card and RESET after a mount failure.

See the [board IO guide](../../../../docs/board-io.md) for pins and reserved functions.
