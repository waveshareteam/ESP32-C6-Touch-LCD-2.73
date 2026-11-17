#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "demo_ui.h"
#include "esp_log.h"

static const char *TAG = "sd_card";
static const char test_text[] = "ESP32-C6-Touch-LCD-2.73 SD test\n";
static const char test_path[] = BSP_SD_MOUNT_POINT "/test.txt";

typedef struct {
    esp_err_t error;
    bsp_sdcard_info_t info;
    bool space_valid;
    const char *status;
    const char *hint;
    char contents[sizeof(test_text)];
} sd_test_result_t;

static void run_sd_test(sd_test_result_t *result)
{
    result->status = "SD card not mounted";
    result->hint = "Insert a FAT32 microSD card,\nthen press RESET to retry.";
    result->error = bsp_sdcard_acquire();
    if (result->error != ESP_OK) {
        ESP_LOGE(TAG, "Mount failed: %s. Insert a FAT32 microSD card and press RESET.",
                 esp_err_to_name(result->error));
        return;
    }
    ESP_LOGI(TAG, "SD card mounted at %s", BSP_SD_MOUNT_POINT);
    result->space_valid = bsp_sdcard_get_info(&result->info) == ESP_OK;

    result->error = ESP_FAIL;
    result->status = "File write failed";
    result->hint = "Check the card is writable,\nthen press RESET to retry.";
    FILE *file = fopen(test_path, "wb");
    if (!file) {
        ESP_LOGE(TAG, "Open for writing failed: %s", strerror(errno));
        goto cleanup;
    }
    size_t bytes = fwrite(test_text, 1, sizeof(test_text) - 1, file);
    int closed = fclose(file);
    if (bytes != sizeof(test_text) - 1 || closed != 0) {
        ESP_LOGE(TAG, "Write or close failed: %s", test_path);
        goto cleanup;
    }
    ESP_LOGI(TAG, "Write successful: %s (%u bytes)", test_path, (unsigned)bytes);

    result->status = "File read failed";
    result->hint = "Check the card, then press\nRESET to repeat the test.";
    file = fopen(test_path, "rb");
    if (!file) {
        ESP_LOGE(TAG, "Open for reading failed: %s", strerror(errno));
        goto cleanup;
    }
    bytes = fread(result->contents, 1, sizeof(result->contents) - 1, file);
    int extra = fgetc(file);
    bool read_error = ferror(file);
    closed = fclose(file);
    result->contents[bytes] = '\0';
    if (read_error || closed != 0 || extra != EOF || bytes != sizeof(test_text) - 1 ||
        memcmp(test_text, result->contents, bytes) != 0) {
        result->contents[0] = '\0';
        ESP_LOGE(TAG, "Read failed or file content did not match: %s", test_path);
        goto cleanup;
    }
    ESP_LOGI(TAG, "Read successful: %s", result->contents);
    result->error = ESP_OK;
    result->status = "Write / read: PASS";
    result->hint = "Test complete.\nPress RESET to run again.";

cleanup:
    ESP_LOGI(TAG, "Write/read verification: %s", esp_err_to_name(result->error));
    bsp_sdcard_release();
    esp_err_t unmount = bsp_sdcard_unmount();
    if (unmount == ESP_OK) {
        ESP_LOGI(TAG, "SD card unmounted; test.txt remains on the card");
    } else {
        ESP_LOGW(TAG, "Unmount failed: %s", esp_err_to_name(unmount));
        if (result->error == ESP_OK) {
            result->error = unmount;
            result->status = "Card cleanup failed";
            result->hint = "Read/write passed. Check\nthe serial log, then RESET.";
        }
    }
}

static void show_result(const sd_test_result_t *result)
{
    if (!demo_start("SD CARD TEST", "microSD / SPI / FAT32")) return;
    demo_lock();
    lv_obj_t *capacity = demo_metric("CAPACITY", 14, 84, 140, 76, &lv_font_montserrat_20);
    lv_obj_t *space = demo_metric("FREE SPACE", 166, 84, 140, 76, &lv_font_montserrat_20);
    if (result->info.mounted) {
        lv_label_set_text_fmt(capacity, "%llu MiB", (unsigned long long)(result->info.capacity_bytes / 1048576));
        if (result->space_valid)
            lv_label_set_text_fmt(space, "%llu MiB", (unsigned long long)(result->info.free_bytes / 1048576));
    }
    lv_obj_t *card = demo_card(14, 172, 292, 98);
    demo_text(card, result->status, 12, 10, 268, &lv_font_montserrat_20,
              result->error == ESP_OK ? 0x16804a : 0xc34040);
    demo_text(card, result->contents[0] ? result->contents : "No verified file data", 12, 56, 268,
              LV_FONT_DEFAULT, 0x63758c);
    demo_status_set(result->error == ESP_OK ? "test.txt verified / RESET to repeat" : result->hint, result->error != ESP_OK);
    demo_unlock();
}

void app_main(void)
{
    sd_test_result_t result = {0};
    // Finish SD access before starting LCD traffic on their shared SPI bus.
    run_sd_test(&result);
    show_result(&result);
}
