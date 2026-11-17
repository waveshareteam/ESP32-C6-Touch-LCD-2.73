# ESP32-C6-Touch-LCD-2.73 Factory Firmware User Guide

[English](README.md) | [简体中文](README_ZH.md)

Connect USB power to explore eleven applications on the 320 × 320 color touchscreen, including display and touch tests, weather, sensors, 2048, a photo badge and a digital clock. This guide explains first use and the controls shown on the screen.

A board running the factory firmware is ready to use. To restore the desktop, follow the [factory firmware instructions](../../firmware/README.md). To build it yourself, see the final section.

## 1. Try the Board for the First Time

1. Connect the board to a computer or USB supply with a USB data cable. Wait for the desktop icons.
2. Open **Display Test** and check the red, green, blue, yellow and white screens. Quickly tap twice to return.
3. Open **Touch Test** and draw with one finger near each corner, along the edges and in the center. Lines should follow your finger.
4. Tap **UI** at the top center. The bottom tools disappear and the button changes to **Exit**; tap **Exit** to return.
5. Swipe left on the desktop, open **Button Test**, then press and release **BOOT**, **LEFT** and **RIGHT**. Each card should show **PASS**, with **3 / 3 buttons passed** in the summary.

Start normally before testing BOOT. Holding BOOT during power-on can enter download mode instead of the desktop.

### Find an Application

The desktop shows up to four icons per page. Swipe horizontally to change pages; the bottom markers show the current page.

| Page | Applications |
| --- | --- |
| 1 | Display Test, Settings, Touch Test, GravityBall |
| 2 | Weather, Sensor, 2048, Button Test |
| 3 | PinsTest, Badge, Watch |

### What You Need

| Feature | Preparation |
| --- | --- |
| Display, touch, buttons, gravity ball, 2048 and sensors | USB power |
| Beijing network time and weather | An Internet-connected 2.4 GHz Wi-Fi network |
| Uploading badge pictures | A phone or computer with a browser, plus JPG, PNG or WebP pictures; no Internet or SD card required |
| Checking SD card capacity | An optional FAT32 microSD card |
| Checking output levels | An optional multimeter, logic analyzer or oscilloscope |

### Return to the Desktop

| Current screen | How to return |
| --- | --- |
| Settings, Weather, Sensor, GravityBall, 2048 or Button Test | Tap the top-left arrow; Settings subpages return to the Settings home first |
| Display Test | Quickly tap twice |
| Touch Test | Tap **UI**, then the resulting **Exit** button |
| Watch, PinsTest or a full-screen badge picture | Swipe up quickly from the bottom center edge |
| Badge upload screen | Select **Done** on the board to leave upload mode before returning |

The bottom-edge swipe also leaves most other applications. Use the dedicated exit controls for Display Test and Touch Test.

## 2. Connect the Board to Wi-Fi

Once online, the firmware requests Beijing time and saves it to the RTC. Weather can also update. **Use 2.4 GHz Wi-Fi; 5 GHz networks are not supported.**

### Set Up Wi-Fi with a Phone or Computer

1. On the board, open **Settings → Wi-Fi setup** and wait for the hotspot name and QR code. Keep this page open during setup.
2. Scan the code or manually join **C6-LCD-2.73-xxxxxx**. Use the actual name on the screen and password **12345678**.
3. If your phone reports no Internet access, choose **Stay connected**. This is the board's local configuration hotspot.
4. Enter **http://192.168.4.1** in the browser's **address bar**, or use the page if it opens automatically.
5. Select **Wi-Fi Setup** on the home page and use the following controls.

| Web control | What to do |
| --- | --- |
| Scan nearby networks | Find nearby networks |
| Nearby networks | Choose your network, or enter its name manually |
| Network name (SSID) | Enter your router's or another phone hotspot's Wi-Fi name |
| Password | Enter that network's password; leave blank for an open network |
| Connect and Save | Submit the settings and wait for the result |

6. **Connected and saved** confirms both connection and storage. The board prefers the saved network after restart.
7. To upload pictures next, select **Device home**, then **Transfer Images**. When finished, select **Disconnect & close hotspot** on the home page and reconnect your phone to its usual network.

**The hotspot can remain available after setup for picture transfers.** Do not expect it to close automatically as on some other products. Setup and badge management share this entry point; if an inactive session expires, reopen Wi-Fi setup or badge management on the board.

**Wi-Fi scan** in Settings only lists nearby networks, signal strength and security. Tapping a network there does not open a password field; use the **Wi-Fi setup** web page to connect.

### Use the Default Hotspot for a Quick Trial

If no network has been saved, enable a 2.4 GHz phone hotspot named **weather-wifi** with password **12345678**, and enable mobile data. The board tries it at startup. If another network is saved, change it through Wi-Fi setup.

### When Does the Time Change?

At startup, the system restores the existing RTC time. Successful network synchronization updates the desktop, Watch and RTC to Beijing time (UTC+8). **Connecting to Wi-Fi and receiving network time are separate steps.** The first correction can take a while; retries can extend the wait to one or two minutes.

The serial message `SNTP synced (UTC+8)` means network time arrived, followed by `Beijing network time saved to RTC` after hardware writeback. Time continues offline and does not revert to the old date. On restart, the saved calendar remains available while RTC power is retained; if the calendar is lost, the firmware uses an initial date until network correction.

## 3. Upload Your Own Badge Pictures

Badge stores **six 320 × 320 pictures** in onboard Flash. It requires neither an SD card nor Internet access. Saved pictures remain after a normal restart.

### Upload Your First Picture

1. Open **Badge** and wait for initialization. First use may prepare storage; wait while messages such as **Preparing your badge...** are shown.
2. Select **Add images**. If a picture is already displayed, tap it to reveal the controls, then select **Manage images**.
3. Join the device hotspot using the QR code or displayed name, with password **12345678**. Open **http://192.168.4.1**, then **Transfer Images**.
4. Select a position under **Pictures**, then use **Choose photo** to open a JPG, PNG or WebP picture.
5. Drag the preview to position the image and adjust **Zoom**. Choose **Background** for uncovered areas. The screen is square: use a square image or keep the subject centered in the preview.
6. Select **Save To Selected Slot** and wait for **Picture saved and active on the device.** Keep the connection and board power on while saving.
7. Choose another position before saving a second picture. Saving to an occupied position replaces its picture.
8. Select **Done** in the browser or on the board to return to viewing.

The browser prepares the required size and color data automatically; **you do not need RGB565, RGB666 or binary conversions**. Color quantization reduces memory requirements, so gradients, small text and photographic details can differ from the original. Use the browser preview as a reference before saving.

### Switch, Manage or Delete Pictures

| Action | How to do it |
| --- | --- |
| Next / previous | Swipe left / right on the board, or tap the picture and use the arrows |
| Hide controls | Select **Hide** or tap the picture again |
| Choose the displayed picture | Select a saved position in the browser, then **Set Active** |
| Delete a picture | Select its position, then **Delete**; only that position is affected |
| Update the browser list | Select **Refresh** |
| Return to the desktop | Leave upload mode, then swipe up from the board's bottom center edge |

Badge is a Flash-based display. This firmware does not include the SD-card Photos gallery, and no `photo` folder is needed.

## 4. Use the Other Applications

### Display Test: Check Screen Colors

The display cycles **red → green → blue → yellow → white**, about **one second per color**. Look for fixed bright or dark points, unusual patches or uneven display. **Quickly tap twice to exit**; shorten the interval if it does not exit.

### Touch Test: Check Drawing and Position

Draw with **one finger** and check that the track is continuous and follows your touch. The bottom tools select black, red, blue, green or orange; **2 / 5 / 9** change stroke width.

The first tap on **UI** hides the tools and changes the button to **Exit**. The second tap returns to the desktop. There is no Clear button: leave and reopen for a fresh canvas. Drawings are not saved to Flash or SD.

### GravityBall: Move the Ball by Tilting

Keep the board flat and still until calibration finishes, then slowly tilt it in different directions. The ball follows the tilt. To recalibrate, place the board flat, press **BOOT** once and wait. Use the top-left arrow to exit.

### Weather: Check the Weather

Connect to Internet-enabled Wi-Fi and open **Weather** for conditions, temperature, humidity and other information. The top-right refresh icon requests an update; the service also refreshes about every **five minutes**.

The default city is **Shenzhen** and does not follow the board's location. Old data can remain after failed requests, so check the update time and status. To change the city, see the final section.

### Sensor: View Three Sensor Groups

| Area | Readings |
| --- | --- |
| PCF85063A / RTC | Hardware clock time and date |
| QMI8658 / MOTION | X/Y/Z acceleration in g; X/Y/Z angular velocity in deg/s |
| SHTC3 | Temperature in °C and relative humidity in %RH |

RTC and environmental values are sampled about once per second; motion updates more frequently. Gravity affects acceleration at rest, so all axes should not be expected to read zero. Angular velocity should be near zero. **RTC OK / IMU OK / SHT OK** indicates successful readings; `--` means a device is unavailable.

Sensor reads the hardware RTC. Watch and the desktop use the system clock restored from RTC and corrected online. These readings should be close after network time is written back.

### 2048: Merge Numbered Tiles

Swipe up, down, left or right inside the board to merge matching numbers, aiming for 2048. **SCORE** is the current score and **BEST** is the highest during this run. **New Game** restarts; the top-left arrow returns to the desktop. Restarting the board clears game state held in memory.

### Button Test: Check Physical Buttons

Press and release **BOOT**, **LEFT** and **RIGHT**. Each card shows **PRESSED / RELEASED**, the **Presses** count and its **PASS** result. Testing all three gives **3 / 3 buttons passed**. **Test again** clears the results.

A held button counts once; release and press again for another count. Test buttons here, and do not press BOOT while PinsTest is driving its shared pin.

### PinsTest: Check Output Levels

This page tests **output levels**; it does not measure voltage or detect external input levels.

| Group | Pins |
| --- | --- |
| GPIO, eight outputs | GPIO9, GPIO3, GPIO15, GPIO23, GPIO13, GPIO12, GPIO22, GPIO21 |
| EXIO, eleven outputs | EXIO4–EXIO14 |

Each item starts at `--`, meaning it has not been operated in this session; **this does not guarantee a low or high-impedance electrical state**. The first tap drives high and shows **H**; the next drives low and shows **L**. Check the output relative to GND with a meter or logic analyzer. The counter shows how many items have been set high in this session.

Swipe up from the bottom center edge to exit. MCU test pins return to input mode; EXIO retains its last levels. GPIO9 is shared with BOOT: do not hold BOOT while driving it, or directly connect a driven pin to another output or a supply rail.

### Watch: Display the Digital Clock

Open **Watch** for large clock digits, date, weekday and seconds. **RTC clock** means the time was restored from hardware; **NTP synced** means network time has been received during this run. The latter can remain after disconnection because it describes the time source, not the current Wi-Fi connection.

Swipe up from the bottom center edge to leave. Watch and the desktop share a global clock; opening Watch does not require a new network request.

## 5. Settings: Brightness, Network and SD Card

Scroll the Settings home page vertically to reach items below the visible area.

| Control | How to use it |
| --- | --- |
| Brightness | Adjust backlight from 10% to 100%; the completed adjustment is saved for startup |
| Wi-Fi scan | View nearby names, signal strength and security; **Scan again** refreshes and arrows change pages |
| Wi-Fi setup | Show hotspot and QR code for browser setup or picture management |
| SD card | View detection status, capacity and free space; select **Refresh** after inserting a card |
| About | View firmware, ESP-IDF version, display size, Flash, PSRAM and MAC information |

The SD card is optional; **Unavailable** without a card does not stop other apps. FAT32 is recommended; back up files before formatting. Capacity uses decimal GB and can differ from computer or packaging values. The board has no PSRAM, so 0 MB in About is expected.

## 6. Troubleshooting

| Symptom | What to try |
| --- | --- |
| No desktop after flashing | Use firmware for this product and press RESET; merged BIN files use 0x0, as in the [flashing guide](../../firmware/README.md) |
| Screen too dim | Increase Brightness in Settings and check power |
| Cannot leave the color loop | Quickly tap twice |
| Touch Test tools disappeared | Tap Exit, then reopen the app |
| Phone reports no Internet on device hotspot | Stay connected; local setup and uploads need no Internet |
| Cannot open 192.168.4.1 | Keep setup/upload open and stay on the hotspot; enter HTTP, not HTTPS or a search term |
| Tapping a Wi-Fi name does not connect | Wi-Fi scan is a list; use the Wi-Fi setup web page |
| Connected, but settings could not be saved | Submit setup again before restarting and wait for Connected and saved |
| Old date after Wi-Fi connects | Check Internet access and wait for NTP; first synchronization may take one or two minutes |
| Beijing time remains after disconnection | Expected: the system and corrected RTC keep time instead of reverting |
| Initial date returns after power-on | Check RTC power retention, then synchronize; complete RTC power loss can lose its calendar |
| Badge keeps preparing on first use | Wait for storage initialization without repeatedly restarting or disconnecting power |
| Badge upload fails or web page stops responding | Stay on the hotspot, reopen Manage images and try JPG/PNG/WebP; select Refresh first to check whether a save completed |
| Badge crop is unsuitable | Adjust position and Zoom, then save over the selected position |
| Badge colors differ from original | Color quantization is used; compare with the browser preview |
| SD card is Unavailable | Expected without a card; otherwise check contact and FAT32, then Refresh |
| GravityBall keeps calibrating | Keep the board flat and still; movement restarts calibration |
| PinsTest shows -- | Tap the item to set its first output; H or L will then appear |

## 7. Build and Flash the Source

Use the [factory merged firmware](../../firmware/README.md) for evaluation without installing a development environment. This directory contains source for **ESP-IDF v5.5.4 and LVGL v9.5.0**, targeting **ESP32-C6, 16 MB Flash and no PSRAM**.

Follow the [ESP-IDF getting-started guide](../../docs/getting-started-idf.md), open an ESP-IDF terminal in this directory, and run:

```sh
idf.py set-target esp32c6
idf.py build
idf.py flash monitor
```

The first build needs Internet to download managed components. Press `Ctrl+]` to leave the serial monitor. To change the weather city, run `idf.py menuconfig`, edit province, city and display name under **C6 2.73 Weather**, then rebuild and flash.

For merged BIN files, follow the [firmware guide](../../firmware/README.md): **address 0x0, ESP32-C6, DIO, 80 MHz, 16 MB**. A standalone application BIN is not a merged image and must not be written directly to 0x0; use `idf.py flash` for source builds.

A full-chip erase or overwriting the relevant data partitions removes saved Wi-Fi, brightness and badge pictures. Configure and upload them again afterward. Erasing onboard Flash does not erase SD card files.
