# Real-Time Clock

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Show the PCF85063 hardware time, date and weekday on the LCD. Connect to Wi-Fi to synchronize Beijing time (UTC+8) over SNTP and save it to the hardware RTC.

## Hardware Required

ESP32-C6-Touch-LCD-2.73, a USB data cable and a 2.4 GHz Wi-Fi network with Internet access for time synchronization.

## How to Run

1. Complete the [Arduino guide](../../../../docs/getting-started-arduino.md) and open `04_I2C_pcf85063.ino`.
2. Enter your network name and password in [`wifi_config.h`](wifi_config.h) using `RTC_WIFI_SSID` and `RTC_WIFI_PASSWORD`.
3. Upload to **ESP32C6 Dev Module**, open Serial Monitor at **115200 baud**, then press RESET.

## Expected Result

Startup reads the existing RTC calendar without resetting it. Once network time arrives, the display switches to Beijing time and saves it to the RTC; the screen refreshes every second and continues to read the RTC offline.

## Troubleshooting

An empty SSID leaves RTC-only mode; check network access if the screen keeps waiting for NTP. If the RTC calendar is invalid, the screen shows `RTC unavailable` until synchronization succeeds; time retention requires continued RTC power.

See the [board IO guide](../../../../docs/board-io.md) for pins and reserved functions.
