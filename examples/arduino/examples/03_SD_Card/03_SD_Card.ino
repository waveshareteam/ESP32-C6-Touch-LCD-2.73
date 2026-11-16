#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <pin_config.h>

void setup()
{
    Serial.begin(115200);
    delay(200);
    pinMode(BOARD_LCD_CS_PIN, OUTPUT);
    digitalWrite(BOARD_LCD_CS_PIN, HIGH);
    SPI.begin(BOARD_LCD_SCLK_PIN, BOARD_LCD_MISO_PIN, BOARD_LCD_MOSI_PIN, BOARD_SD_CS_PIN);
    if (!SD.begin(BOARD_SD_CS_PIN, SPI, 20000000)) {
        Serial.println("SD mount failed. Insert a FAT32 card and press RESET.");
        return;
    }
    Serial.printf("SD capacity: %llu MiB; used: %llu MiB\n",
                  SD.cardSize() / (1024 * 1024), SD.usedBytes() / (1024 * 1024));
    const char text[] = "ESP32-C6-Touch-LCD-2.73 SD test\n";
    const char path[] = "/test.txt";
    SD.remove(path);
    File file = SD.open(path, FILE_WRITE);
    if (!file) { Serial.println("File open for write failed"); SD.end(); return; }
    bool ok = file.write((const uint8_t *)text, sizeof(text) - 1) == sizeof(text) - 1;
    file.close();
    file = SD.open(path, FILE_READ);
    String actual = file ? file.readString() : String();
    ok = ok && file && actual == text;
    file.close();
    Serial.println(ok ? "SD write/read: PASS" : "SD write/read: FAIL");
    Serial.print(actual);
    SD.end();
}

void loop() { delay(1000); }
