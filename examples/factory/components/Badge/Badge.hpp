/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include "systems/phone/esp_brookesia_phone_app.hpp"
#include "app_support.hpp"
#include "badge_service.hpp"

namespace esp_brookesia::apps {
class Badge final : public systems::phone::App, private AppWorker {
public:
    static Badge *requestInstance();
    ~Badge() override;
protected:
    bool init() override { return true; }
    bool run() override;
    bool back() override;
    bool close() override;
    bool deinit() override { return close(); }
    bool pause() override;
    bool resume() override;
private:
    Badge();
    void work() override;
    void refresh();
    void show_controls(bool show);
    void open_editor();
    void end_editor();
    void change_image(int direction);
    static void event(lv_event_t *);
    static void touch(lv_event_t *);
    static void poll(lv_timer_t *);
    static bool cancel(void *p) { return static_cast<Badge *>(p)->stopping(); }
    static Badge *instance_;
    SemaphoreHandle_t mutex_=nullptr;
    std::atomic<int> direction_{0};
    std::atomic<bool> selection_done_{false};
    std::atomic<uint32_t> requested_{0};
    uint32_t completed_=0, displayed_revision_=0, wanted_revision_=0;
    int wanted_slot_=0;
    bool loaded_=false, changing_=false, editing_=false, editor_failed_=false, portal_seen_=false;
    bool touch_active_=false, controls_visible_=false;
    lv_point_t touch_start_={};
    uint32_t touch_tick_=0;
    int64_t editor_started_=0;
    char failure_[96]={};
    uint8_t *pixels_=nullptr;
    badge::State state_={};
    lv_image_dsc_t image_={};
    lv_obj_t *root_=nullptr,*image_obj_=nullptr,*welcome_=nullptr,*welcome_text_=nullptr;
    lv_obj_t *add_=nullptr,*message_=nullptr,*controls_=nullptr,*counter_=nullptr;
    lv_obj_t *previous_=nullptr,*next_=nullptr,*editor_=nullptr,*connection_=nullptr;
    lv_obj_t *qr_=nullptr;
    char qr_ssid_[33]={};
    lv_timer_t *timer_=nullptr;
};
}
