#include <Demo_UI.h>
#include <WiFi.h>
#include <esp_sntp.h>
#include <atomic>
#include <time.h>
#include "rtc_bsp.h"
#include "wifi_config.h"

static lv_obj_t *clock_label, *date_label, *day_label;
static bool screen_ready, rtc_ready, ntp_started, sync_due, synced;
static uint32_t last_sample, last_connect, last_write;
static std::atomic<bool> sync_pending{false};
static void network_time_received(struct timeval *) { sync_pending.store(true); }

void setup()
{
    Serial.begin(115200);
    delay(200);
    screen_ready = demo_start("BEIJING RTC CLOCK", "PCF85063 / Wi-Fi + SNTP / UTC+8");
    if (!screen_ready) return;
    lv_obj_t *card = demo_card(14, 84, 292, 172);
    demo_text(card, "RTC TIME / BEIJING", 16, 14, 260, LV_FONT_DEFAULT, 0x63758c);
    clock_label = demo_text(card, "--:--:--", 16, 47, 260, &lv_font_montserrat_28, 0x17365c);
    date_label = demo_text(card, "---- / -- / --", 16, 94, 260, &lv_font_montserrat_20, 0x17365c);
    day_label = demo_text(card, "--", 16, 130, 260, LV_FONT_DEFAULT, 0x63758c);
    demo_text(demo_screen(), "RTC keeps time while Wi-Fi is offline", 14, 262, 292, LV_FONT_DEFAULT, 0x63758c);
    rtc_ready = rtc_init();
    RtcDateTime_t first = {};
    if (rtc_ready && i2c_rtc_get(first)) {
        lv_label_set_text_fmt(clock_label, "%02u:%02u:%02u",first.hour,first.minute,first.second);
        lv_label_set_text_fmt(date_label, "%04u / %02u / %02u",first.year,first.month,first.day);
        lv_refr_now(nullptr);
    }
    setenv("TZ", RTC_TIMEZONE, 1);
    tzset();
    if (RTC_WIFI_SSID[0]) {
        WiFi.persistent(false);
        WiFi.mode(WIFI_STA);
        WiFi.setAutoReconnect(true);
        WiFi.begin(RTC_WIFI_SSID, RTC_WIFI_PASSWORD);
    }
    last_sample = millis() - 1000;
    last_connect = millis();
    last_write = millis() - 5000;
}
void loop()
{
    demo_tick();
    if (screen_ready && millis() - last_sample >= 1000) {
        const uint32_t now = millis();
        last_sample = now;
        if (!rtc_ready) rtc_ready = rtc_init();
        const bool online = RTC_WIFI_SSID[0] && WiFi.isConnected();
        if (RTC_WIFI_SSID[0] && !online && now - last_connect >= 10000) {
            last_connect = now;
            WiFi.reconnect();
        }
        if (online && !ntp_started) {
            sntp_set_time_sync_notification_cb(network_time_received);
            configTzTime(RTC_TIMEZONE, RTC_NTP_SERVER, "pool.ntp.org");
            ntp_started = true;
        }
        if (sync_pending.exchange(false)) sync_due = true;
        if (sync_due && rtc_ready && now - last_write >= 5000) {
            last_write = now;
            time_t epoch = time(nullptr);
            struct tm local;
            localtime_r(&epoch, &local);
            if (i2c_rtc_setTime(local.tm_year+1900,local.tm_mon+1,local.tm_mday,
                                local.tm_hour,local.tm_min,local.tm_sec)) {
                sync_due = false;
                synced = true;
                Serial.println("SNTP Beijing time saved to hardware RTC");
            }
        }
        RtcDateTime_t time = {};
        const bool valid = rtc_ready && i2c_rtc_get(time);
        if (valid) {
            const char *days[] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
            lv_label_set_text_fmt(clock_label, "%02u:%02u:%02u",time.hour,time.minute,time.second);
            lv_label_set_text_fmt(date_label, "%04u / %02u / %02u",time.year,time.month,time.day);
            lv_label_set_text(day_label, days[time.week]);
            Serial.printf("%04u-%02u-%02u %02u:%02u:%02u\n",time.year,time.month,time.day,time.hour,time.minute,time.second);
        } else {
            lv_label_set_text(clock_label, "--:--:--");
            lv_label_set_text(date_label, "RTC unavailable");
            lv_label_set_text(day_label, "Waiting for network time");
        }
        const char *message = !RTC_WIFI_SSID[0] ? "Set Wi-Fi in wifi_config.h" :
            sync_due ? "RTC sync write pending / retrying" :
            !online ? "Wi-Fi offline / reconnecting" :
            synced ? "Beijing time saved to RTC" : "Wi-Fi ready / waiting for NTP";
        demo_status_set(message, !valid || sync_due);
    }
    delay(5);
}
