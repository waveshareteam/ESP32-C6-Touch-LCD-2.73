#include "demo_ui.h"
#include "wifi_config.h"
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static atomic_bool connected, sync_pending;
static void network_event(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg; (void)data;
    if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) atomic_store(&connected, true);
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) atomic_store(&connected, false);
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) esp_wifi_connect();
}
static void network_time_received(struct timeval *time)
{
    (void)time;
    // The application task writes the calendar; never access I2C/LVGL from this callback.
    atomic_store(&sync_pending, true);
}
static esp_err_t start_network(void)
{
    if (!RTC_WIFI_SSID[0]) return ESP_ERR_NOT_FOUND;
    const size_t ssid_length = strlen(RTC_WIFI_SSID), password_length = strlen(RTC_WIFI_PASSWORD);
    wifi_config_t config = {0};
    if (ssid_length > sizeof(config.sta.ssid) || password_length >= sizeof(config.sta.password))
        return ESP_ERR_INVALID_ARG;
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_OK) ret = bsp_wifi_init();
    if (ret == ESP_OK) ret = esp_wifi_set_storage(WIFI_STORAGE_RAM);
    if (ret == ESP_OK) ret = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, network_event, NULL);
    if (ret == ESP_OK) ret = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, network_event, NULL);
    memcpy(config.sta.ssid, RTC_WIFI_SSID, ssid_length);
    memcpy(config.sta.password, RTC_WIFI_PASSWORD, password_length);
    if (ret == ESP_OK) ret = esp_wifi_set_config(WIFI_IF_STA, &config);
    if (ret == ESP_OK) ret = bsp_wifi_start();
    return ret;
}
void app_main(void)
{
    setenv("TZ", RTC_TIMEZONE, 1);
    tzset();
    ESP_ERROR_CHECK(demo_start("BEIJING RTC CLOCK", "PCF85063 / Wi-Fi + SNTP / UTC+8") ? ESP_OK : ESP_FAIL);
    demo_lock();
    lv_obj_t *card = demo_card(14, 84, 292, 172);
    demo_text(card, "RTC TIME / BEIJING", 16, 14, 260, LV_FONT_DEFAULT, 0x63758c);
    lv_obj_t *clock = demo_text(card, "--:--:--", 16, 47, 260, &lv_font_montserrat_28, 0x17365c);
    lv_obj_t *date = demo_text(card, "---- / -- / --", 16, 94, 260, &lv_font_montserrat_20, 0x17365c);
    lv_obj_t *day = demo_text(card, "--", 16, 130, 260, LV_FONT_DEFAULT, 0x63758c);
    demo_text(demo_screen(), "RTC keeps time while Wi-Fi is offline", 14, 262, 292, LV_FONT_DEFAULT, 0x63758c);
    demo_unlock();
    // Initialize/read the RTC before starting Wi-Fi. Do not overwrite a valid calendar.
    bool rtc_ready = bsp_rtc_init() == ESP_OK;
    bsp_sensor_datetime_t first = {0};
    if (rtc_ready && bsp_rtc_get_datetime(&first) == ESP_OK) {
        demo_lock();
        lv_label_set_text_fmt(clock, "%02u:%02u:%02u", first.hour,first.minute,first.second);
        lv_label_set_text_fmt(date, "%04u / %02u / %02u", first.year,first.month,first.day);
        demo_unlock();
    }
    esp_err_t network = start_network();
    if (network != ESP_OK && network != ESP_ERR_NOT_FOUND)
        ESP_LOGW("rtc", "Wi-Fi initialization: %s", esp_err_to_name(network));
    bool sntp_started = false, sync_due = false, synced = false;
    unsigned ticks = 0, last_write_tick = 0;
    const char *days[] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
    while (true) {
        ++ticks;
        if (!rtc_ready) rtc_ready = bsp_rtc_init() == ESP_OK;
        const bool online = atomic_load(&connected);
        if (network == ESP_OK && !online && ticks % 10 == 0) esp_wifi_connect();
        if (online && !sntp_started) {
            esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
            esp_sntp_setservername(0, RTC_NTP_SERVER);
            esp_sntp_setservername(1, "pool.ntp.org");
            sntp_set_time_sync_notification_cb(network_time_received);
            esp_sntp_init();
            sntp_started = true;
        }
        if (atomic_exchange(&sync_pending, false)) sync_due = true;
        if (sync_due && rtc_ready && (last_write_tick == 0 || ticks - last_write_tick >= 5)) {
            last_write_tick = ticks;
            time_t now = time(NULL);
            struct tm local;
            localtime_r(&now, &local);
            const bsp_sensor_datetime_t corrected = {.year=local.tm_year+1900,.month=local.tm_mon+1,
                .day=local.tm_mday,.hour=local.tm_hour,.minute=local.tm_min,.second=local.tm_sec,.weekday=local.tm_wday};
            esp_err_t ret = bsp_rtc_set_datetime(&corrected);
            if (ret == ESP_OK) {
                sync_due = false;
                synced = true;
                ESP_LOGI("rtc", "SNTP Beijing time saved to hardware RTC");
            } else ESP_LOGW("rtc", "RTC synchronization write: %s", esp_err_to_name(ret));
        }
        bsp_sensor_datetime_t time = {0};
        esp_err_t ret = rtc_ready ? bsp_rtc_get_datetime(&time) : ESP_ERR_INVALID_STATE;
        demo_lock();
        if (ret == ESP_OK) {
            lv_label_set_text_fmt(clock, "%02u:%02u:%02u", time.hour,time.minute,time.second);
            lv_label_set_text_fmt(date, "%04u / %02u / %02u", time.year,time.month,time.day);
            lv_label_set_text(day, days[time.weekday]);
        } else {
            lv_label_set_text(clock, "--:--:--");
            lv_label_set_text(date, "RTC unavailable");
            lv_label_set_text(day, "Waiting for RTC");
        }
        const char *message = !RTC_WIFI_SSID[0] ? "Set Wi-Fi in wifi_config.h" :
            network != ESP_OK ? "Wi-Fi init failed / check serial log" :
            sync_due ? "RTC sync write pending / retrying" :
            !online ? "Wi-Fi offline / reconnecting" :
            synced ? "Beijing time saved to RTC" : "Wi-Fi ready / waiting for NTP";
        demo_status_set(message, ret != ESP_OK || (network != ESP_OK && network != ESP_ERR_NOT_FOUND) || sync_due);
        demo_unlock();
        if (ret == ESP_OK) ESP_LOGI("rtc", "%04u-%02u-%02u %02u:%02u:%02u",time.year,time.month,time.day,time.hour,time.minute,time.second);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
