# Factory Application

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

A 320 × 320 desktop with eleven apps: Display Test, Settings, Touch Test, Ball, Weather, Sensor, 2048, Button Test, PinsTest, Badge and Watch. Badge stores six images in Flash; the SD-card Photos gallery is not included.

## Hardware Required

ESP32-C6-Touch-LCD-2.73 and a USB data cable. A microSD card is optional for checking its capacity in Settings; the application does not require audio hardware or PSRAM.

## How to Run

Use ESP-IDF v5.5.4 in this directory; the Component Manager downloads dependencies automatically.

```sh
idf.py set-target esp32c6
idf.py build
idf.py flash monitor
```

## Expected Result

The desktop shows four icons per page; swipe horizontally to change pages and swipe up from the bottom center to leave most apps. Display Test exits on a double tap; in Touch Test, tap UI to hide the tools, then tap Exit to return.

Open Settings → Wi-Fi setup or Badge → Add/Manage images, join the device hotspot with password `12345678`, and open **http://192.168.4.1** for Wi-Fi setup or badge uploads. Watch and the desktop start from a valid RTC calendar; Internet time corrects both the system clock and RTC, then time continues offline.

## Troubleshooting

Only 2.4 GHz Wi-Fi is supported; without saved credentials the factory application tries SSID `weather-wifi` with password `12345678`, and a different network can be selected in Settings. Badge works without Internet access or a microSD card; a missing card is reported in Settings.

PinsTest initially shows `--` until a pin is operated: the first tap outputs high and subsequent taps alternate high/low. It tests GPIO9/3/15/23/13/12/22/21 and EXIO4–14; MCU pins return to input when the app closes.
