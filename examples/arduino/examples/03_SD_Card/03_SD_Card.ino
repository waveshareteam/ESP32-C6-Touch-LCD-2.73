#include <Demo_UI.h>
#include <SPI.h>
#include <SD.h>

struct CardResult {
    bool mounted = false, passed = false, space_valid = false;
    uint64_t capacity = 0, free_bytes = 0;
    String status = "SD card not mounted";
    String contents;
};
static CardResult test_card();
static CardResult test_card()
{
    CardResult result;
    // Finish card access and release Arduino SPI before the LCD takes SPI2.
    pinMode(BOARD_LCD_CS_PIN, OUTPUT);
    digitalWrite(BOARD_LCD_CS_PIN, HIGH);
    SPI.begin(BOARD_LCD_SCLK_PIN, BOARD_LCD_MISO_PIN, BOARD_LCD_MOSI_PIN, BOARD_SD_CS_PIN);
    if (SD.begin(BOARD_SD_CS_PIN, SPI, 20000000)) {
        result.mounted = true;
        result.capacity = SD.cardSize();
        const uint64_t total = SD.totalBytes(), used = SD.usedBytes();
        result.space_valid = total > 0 && used <= total;
        result.free_bytes = result.space_valid ? total - used : 0;
        const char path[] = "/test.txt";
        const char text[] = "ESP32-C6-Touch-LCD-2.73 SD test\n";
        bool removed = !SD.exists(path) || SD.remove(path);
        File file = removed ? SD.open(path, FILE_WRITE) : File();
        bool written = file && file.write((const uint8_t *)text, sizeof(text)-1) == sizeof(text)-1;
        if (file) { file.flush(); written = written && !file.getWriteError(); file.close(); }
        if (written) {
            file = SD.open(path, FILE_READ);
            if (file) { result.contents = file.readString(); result.passed = result.contents == text; file.close(); }
            result.status = result.passed ? "Write / read: PASS" : "File verify failed";
        } else result.status = "File write failed";
    }
    SD.end();
    SPI.end();
    return result;
}
void setup()
{
    Serial.begin(115200);
    delay(200);
    const CardResult result = test_card();
    Serial.println(result.status);
    Serial.printf("Capacity: %llu MiB; free: %llu MiB\n",result.capacity/1048576,result.free_bytes/1048576);
    Serial.print(result.contents);
    if (!demo_start("SD CARD TEST", "microSD / SPI / FAT32")) return;
    lv_obj_t *capacity = demo_metric("CAPACITY", 14, 84, 140, 76, &lv_font_montserrat_20);
    lv_obj_t *space = demo_metric("FREE SPACE", 166, 84, 140, 76, &lv_font_montserrat_20);
    if (result.mounted) {
        lv_label_set_text_fmt(capacity, "%llu MiB",result.capacity/1048576);
        if (result.space_valid) lv_label_set_text_fmt(space, "%llu MiB",result.free_bytes/1048576);
    }
    lv_obj_t *card = demo_card(14, 172, 292, 98);
    demo_text(card, result.status.c_str(), 12, 10, 268, &lv_font_montserrat_20,
              result.passed ? 0x16804a : 0xc34040);
    demo_text(card, result.passed ? result.contents.c_str() : "No verified file data", 12, 56, 268,
              LV_FONT_DEFAULT, 0x63758c);
    demo_status_set(result.passed ? "test.txt verified / RESET to repeat" :
                    "Check the FAT32 card; RESET to retry", !result.passed);
}
void loop() { demo_tick(); delay(5); }
