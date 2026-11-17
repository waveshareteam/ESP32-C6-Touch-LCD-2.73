/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include "systems/phone/esp_brookesia_phone_app.hpp"
#include "wifi_manager.h"

namespace esp_brookesia::apps {
class Watch final : public systems::phone::App {
public:
    static Watch *requestInstance();
    ~Watch() override;
protected:
    bool init() override { return true; }
    bool run() override;
    bool back() override { return notifyCoreClosed(); }
    bool close() override;
    bool deinit() override { return close(); }
    bool pause() override;
    bool resume() override;
private:
    Watch();
    void refresh();
    static void draw_digits(lv_event_t *);
    static void event(lv_event_t *);
    static void tick(lv_timer_t *);
    static Watch *instance_;
    lv_obj_t *root_=nullptr, *digits_=nullptr, *date_=nullptr, *weekday_=nullptr;
    lv_obj_t *seconds_=nullptr, *source_=nullptr;
    lv_timer_t *timer_=nullptr;
    int values_[4]={-1,-1,-1,-1};
    time_t last_second_=(time_t)-1;
    wifi_clock_source_t last_source_=WIFI_CLOCK_UNSET;
    bool valid_=false, colon_=true;
};
}
