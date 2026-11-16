# Real-Time Clock

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Show the PCF85063 hardware time, date and weekday on the LCD. Connect to Wi-Fi to synchronize Beijing time (UTC+8) over SNTP and save it to the hardware RTC.

## Hardware Required

ESP32-C6-Touch-LCD-2.73, a USB data cable and a 2.4 GHz Wi-Fi network with Internet access for time synchronization.

## How to Run

1. Complete the [ESP-IDF guide](../../../docs/getting-started-idf.md) and open this project in an ESP-IDF v5.5.4 terminal.
2. Enter your network name and password in [`wifi_config.h`](main/wifi_config.h) using `RTC_WIFI_SSID` and `RTC_WIFI_PASSWORD`.
3. Run `idf.py build`, then `idf.py flash monitor`.

## Expected Result

Startup reads the existing RTC calendar without resetting it. Once network time arrives, the display switches to Beijing time and saves it to the RTC; the screen refreshes every second and continues to read the RTC offline.

## Troubleshooting

An empty SSID leaves RTC-only mode; check network access if the screen keeps waiting for NTP. An invalid RTC calendar is initialized by the BSP to 2025-01-01 12:00:00 before network synchronization; RTC communication errors show `RTC unavailable`. Time retention requires continued RTC power.

See the [board IO guide](../../../docs/board-io.md) for pins and reserved functions.
