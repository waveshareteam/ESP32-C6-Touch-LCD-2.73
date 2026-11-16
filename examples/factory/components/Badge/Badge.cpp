/* SPDX-License-Identifier: Apache-2.0 */
#include "Badge.hpp"
#include "wifi_manager.h"
#include "esp_timer.h"
#include "src/misc/cache/instance/lv_image_cache.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>

LV_IMAGE_DECLARE(img_app_badge);
namespace {
constexpr uint32_t BG=0xf5f3ed, INK=0x283b40, GREEN=0x496c61, MUTED=0x788b86, SOFT=0xe7ece5;
constexpr size_t IMAGE_BYTES=256*sizeof(lv_color32_t)+320*320;
void centered(lv_obj_t *o) { lv_obj_set_style_text_align(o,LV_TEXT_ALIGN_CENTER,0); }
lv_obj_t *badge_button(lv_obj_t *p,const char *text,int x,int y,int w,int h,int id,lv_event_cb_t cb,void *ctx,bool primary=false)
{
    auto *b=app_ui::button(p,text,x,y,w,h,id,cb,ctx);
    lv_obj_set_style_bg_color(b,lv_color_hex(primary?GREEN:SOFT),0);
    lv_obj_set_style_bg_color(b,lv_color_hex(primary?0x345447:0xd7e0d6),LV_STATE_PRESSED);
    lv_obj_set_style_text_color(lv_obj_get_child(b,0),lv_color_hex(primary?0xffffff:GREEN),0);
    return b;
}
}
namespace esp_brookesia::apps {
Badge *Badge::instance_;
Badge *Badge::requestInstance() { if (!instance_) instance_=new(std::nothrow) Badge; return instance_; }
Badge::Badge():App("Badge",&img_app_badge,true,false,false) {}
Badge::~Badge() { close(); if (mutex_) vSemaphoreDelete(mutex_); instance_=nullptr; }
bool Badge::run()
{
    if (!mutex_) mutex_=xSemaphoreCreateMutex();
    if (!mutex_) return false;
    state_={}; requested_=0; direction_=0; selection_done_=false; completed_=0; displayed_revision_=wanted_revision_=0;
    loaded_=changing_=editing_=editor_failed_=touch_active_=controls_visible_=false;
    root_=app_ui::box(lv_screen_active(),0,0,320,320,BG,0);
    lv_obj_add_flag(root_,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_remove_flag(root_,LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_add_event_cb(root_,touch,LV_EVENT_ALL,this);
    image_obj_=lv_image_create(root_);
    lv_obj_remove_flag(image_obj_,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(image_obj_,LV_OBJ_FLAG_HIDDEN);
    welcome_=app_ui::box(root_,0,0,320,320,BG,0);
    badge_button(welcome_,LV_SYMBOL_LEFT,12,12,32,32,0,event,this);
    app_ui::label(welcome_,"PHOTO BADGE",56,21,238,&lv_font_montserrat_14,GREEN);
    // A small illustration leaves room for a readable square-screen layout.
    auto *art=app_ui::box(welcome_,118,60,84,68,SOFT,16);
    app_ui::label(art,LV_SYMBOL_IMAGE,20,15,54,&lv_font_montserrat_40,GREEN);
    auto *title=app_ui::label(welcome_,"Make it yours",16,148,288,&lv_font_montserrat_28,INK); centered(title);
    welcome_text_=app_ui::label(welcome_,"Preparing your badge...",24,190,272,&lv_font_montserrat_14,MUTED); centered(welcome_text_);
    lv_obj_set_height(welcome_text_,44); lv_label_set_long_mode(welcome_text_,LV_LABEL_LONG_DOT);
    add_=badge_button(welcome_,LV_SYMBOL_PLUS "  Add images",56,244,208,42,1,event,this,true);
    app_ui::enabled(add_,false);
    auto *hint=app_ui::label(welcome_,"6 favorites / saved on your device",16,299,288,&lv_font_montserrat_12,MUTED); centered(hint);
    message_=app_ui::label(root_,"Loading...",16,137,288,&lv_font_montserrat_16,INK); centered(message_);
    lv_obj_set_style_bg_color(message_,lv_color_hex(BG),0); lv_obj_set_style_bg_opa(message_,LV_OPA_COVER,0);
    lv_obj_set_style_pad_all(message_,8,0); lv_obj_add_flag(message_,LV_OBJ_FLAG_HIDDEN);
    controls_=app_ui::box(root_,8,191,304,121,BG,18);
    lv_obj_add_flag(controls_,LV_OBJ_FLAG_CLICKABLE);
    previous_=badge_button(controls_,LV_SYMBOL_LEFT,10,12,40,34,3,event,this);
    next_=badge_button(controls_,LV_SYMBOL_RIGHT,254,12,40,34,4,event,this);
    counter_=app_ui::label(controls_,"",58,20,188,&lv_font_montserrat_14,INK); centered(counter_);
    badge_button(controls_,"Manage images",10,61,180,44,1,event,this,true);
    badge_button(controls_,"Hide",202,61,92,44,2,event,this);
    lv_obj_add_flag(controls_,LV_OBJ_FLAG_HIDDEN);
    if (!start("badge",5120,2)) { close(); return false; }
    timer_=lv_timer_create(poll,100,this);
    if (!timer_) { close(); return false; }
    return true;
}
void Badge::work()
{
    if (!badge::request_initialize()) return;
    uint32_t handled=0;
    while (!stopping()) {
        badge::State state;
        if (!badge::snapshot(state) || !state.ready) { vTaskDelay(pdMS_TO_TICKS(30)); continue; }
        const int direction=direction_.exchange(0);
        if (direction) { badge::next(direction,cancel,this); selection_done_=true; }
        const uint32_t request=requested_.load();
        if (request && request!=handled) {
            xSemaphoreTake(mutex_,portMAX_DELAY);
            const int slot=wanted_slot_;
            xSemaphoreGive(mutex_);
            char failure[96]={};
            const bool ok=badge::load(slot,pixels_,failure,sizeof(failure),cancel,this);
            xSemaphoreTake(mutex_,portMAX_DELAY);
            if (request==requested_.load()) {
                completed_=request; loaded_=ok;
                std::snprintf(failure_,sizeof(failure_),"%s",failure);
            }
            xSemaphoreGive(mutex_);
            handled=request;
        }
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}
void Badge::refresh()
{
    badge::State current;
    // Sample completion first: a successful snapshot after this observation
    // includes the committed selection, even if HTTP held the lock earlier.
    const bool selection_finished=selection_done_.load();
    if (badge::snapshot(current)) {
        state_=current;
        if (selection_finished) { selection_done_=false; changing_=false; }
    }
    if (!state_.ready || !state_.available) {
        lv_obj_add_flag(image_obj_,LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(welcome_,LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(message_,LV_OBJ_FLAG_HIDDEN);
        show_controls(false);
        const char *initializing=badge::initialization_status();
        app_ui::text(welcome_text_,initializing?initializing:state_.error[0]?state_.error:state_.ready?
                     "Crop and upload from your phone.\nNo SD card needed.":"Preparing your badge...");
        app_ui::enabled(add_,state_.ready);
    } else if (state_.revision!=wanted_revision_) {
        // One indexed frame is reused only while hidden from the renderer.
        lv_obj_add_flag(image_obj_,LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(welcome_,LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(message_,LV_OBJ_FLAG_HIDDEN);
        app_ui::text(message_,"Loading...");
        if (!pixels_) pixels_=static_cast<uint8_t *>(heap_caps_aligned_alloc(LV_DRAW_BUF_ALIGN,IMAGE_BYTES,MALLOC_CAP_8BIT));
        if (!pixels_) app_ui::text(message_,"Not enough memory.\nExit and try again.");
        else {
            lv_image_cache_drop(&image_);
            image_={}; image_.header.magic=LV_IMAGE_HEADER_MAGIC; image_.header.cf=LV_COLOR_FORMAT_I8;
            image_.header.w=320; image_.header.h=320; image_.header.stride=320;
            image_.data_size=IMAGE_BYTES; image_.data=pixels_;
            xSemaphoreTake(mutex_,portMAX_DELAY);
            wanted_slot_=state_.active; wanted_revision_=state_.revision;
            requested_.fetch_add(1);
            xSemaphoreGive(mutex_);
            changing_=false;
        }
    }
    if (pixels_ && state_.ready && state_.available && !changing_) {
        // Loading and storage never wait on the GUI lock.
        if (xSemaphoreTake(mutex_,0)==pdTRUE) {
            if (completed_==requested_.load() && completed_) {
                if (loaded_ && displayed_revision_!=wanted_revision_) {
                    lv_image_cache_drop(&image_); lv_image_set_src(image_obj_,&image_);
                    lv_obj_invalidate(image_obj_); displayed_revision_=wanted_revision_;
                }
                if (loaded_) {
                    lv_obj_remove_flag(image_obj_,LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(message_,LV_OBJ_FLAG_HIDDEN);
                } else {
                    app_ui::text(message_,failure_[0]?failure_:"Image unavailable");
                    lv_obj_remove_flag(message_,LV_OBJ_FLAG_HIDDEN);
                }
            }
            xSemaphoreGive(mutex_);
        }
    }
    int count=0, index=0;
    for (int i=0;i<badge::SLOTS;++i) if (state_.available&(1U<<i)) { ++count; if (i==state_.active) index=count; }
    char text[160]; std::snprintf(text,sizeof(text),"Image %d of %d",index,count); app_ui::text(counter_,text);
    app_ui::enabled(previous_,count>1); app_ui::enabled(next_,count>1);
    if (!editing_ || editor_failed_) return;
    if (badge::editor_finished()) { end_editor(); return; }
    wifi_manager_snapshot_t wifi={}; wifi_manager_get_snapshot(&wifi);
    if (wifi.provisioning) {
        portal_seen_=true;
        if (std::strcmp(qr_ssid_,wifi.ap_ssid)) {
            char payload[128]; std::snprintf(payload,sizeof(payload),"WIFI:T:WPA;S:%s;P:%s;;",wifi.ap_ssid,WIFI_PROVISION_PASSWORD);
            if (lv_qrcode_update(qr_,payload,std::strlen(payload))==LV_RESULT_OK)
                std::snprintf(qr_ssid_,sizeof(qr_ssid_),"%s",wifi.ap_ssid);
        }
        std::snprintf(text,sizeof(text),"%s\nPassword: %s\n192.168.4.1 > Photo Badge",wifi.ap_ssid,WIFI_PROVISION_PASSWORD);
    } else if (portal_seen_ || esp_timer_get_time()-editor_started_>15000000)
        std::snprintf(text,sizeof(text),"Hotspot unavailable.\nTap Done and try again.");
    else std::snprintf(text,sizeof(text),"Starting hotspot...");
    app_ui::text(connection_,text);
}
void Badge::show_controls(bool show)
{
    controls_visible_=show;
    if (show) lv_obj_remove_flag(controls_,LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(controls_,LV_OBJ_FLAG_HIDDEN);
}
void Badge::change_image(int direction)
{
    unsigned count=0;
    for (unsigned i=0;i<badge::SLOTS;++i) if (state_.available&(1U<<i)) ++count;
    if (count<2 || changing_) return;
    lv_obj_add_flag(image_obj_,LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(message_,LV_OBJ_FLAG_HIDDEN); app_ui::text(message_,"Loading...");
    changing_=true; selection_done_=false; direction_=direction;
}
void Badge::open_editor()
{
    if (editing_) return;
    show_controls(false);
    editor_=app_ui::box(root_,0,0,320,320,BG,0);
    lv_obj_add_flag(editor_,LV_OBJ_FLAG_CLICKABLE);
    auto *title=app_ui::label(editor_,"Add your moments",16,16,288,&lv_font_montserrat_22,INK); centered(title);
    qr_=lv_qrcode_create(editor_);
    lv_qrcode_set_size(qr_,136); lv_qrcode_set_dark_color(qr_,lv_color_black());
    lv_qrcode_set_light_color(qr_,lv_color_white()); lv_qrcode_set_quiet_zone(qr_,true);
    lv_obj_set_pos(qr_,92,56); qr_ssid_[0]=0;
    connection_=app_ui::label(editor_,"Starting hotspot...",16,205,288,&lv_font_montserrat_14,INK); centered(connection_);
    lv_obj_set_height(connection_,62); lv_label_set_long_mode(connection_,LV_LABEL_LONG_DOT);
    badge_button(editor_,"Done",80,276,160,34,5,event,this,true);
    editing_=true; portal_seen_=false; editor_started_=esp_timer_get_time();
    editor_failed_=!badge::start_editor();
    if (editor_failed_) {
        lv_obj_add_flag(qr_,LV_OBJ_FLAG_HIDDEN);
        app_ui::text(connection_,"Wi-Fi is busy.\nTap Done and try again.");
    }
    refresh();
}
void Badge::end_editor()
{
    if (!editor_failed_) badge::stop_editor();
    editing_=false; editor_failed_=false;
    if (editor_) lv_obj_delete(editor_);
    editor_=connection_=qr_=nullptr;
    refresh();
}
void Badge::event(lv_event_t *event)
{
    auto *self=static_cast<Badge *>(lv_event_get_user_data(event));
    switch (app_ui::action(event)) {
    case 0: self->back(); return;
    case 1: self->open_editor(); return;
    case 2: self->show_controls(false); return;
    case 3: self->change_image(-1); return;
    case 4: self->change_image(1); return;
    case 5: self->end_editor(); return;
    }
}
void Badge::touch(lv_event_t *event)
{
    auto *self=static_cast<Badge *>(lv_event_get_user_data(event));
    auto *input=lv_event_get_indev(event);
    if (!input || self->editing_) return;
    const auto code=lv_event_get_code(event);
    if (code==LV_EVENT_PRESSED) {
        lv_indev_get_point(input,&self->touch_start_); self->touch_tick_=lv_tick_get(); self->touch_active_=true;
    } else if (code==LV_EVENT_PRESS_LOST) self->touch_active_=false;
    else if (code==LV_EVENT_RELEASED && self->touch_active_) {
        self->touch_active_=false;
        lv_point_t point; lv_indev_get_point(input,&point);
        const int dx=point.x-self->touch_start_.x,dy=point.y-self->touch_start_.y;
        if (std::abs(dx)>=50 && std::abs(dx)>std::abs(dy)*2) self->change_image(dx<0?1:-1);
        else if (std::abs(dx)<12 && std::abs(dy)<12 && lv_tick_elaps(self->touch_tick_)<500 && self->state_.available)
            self->show_controls(!self->controls_visible_);
    }
}
void Badge::poll(lv_timer_t *timer) { static_cast<Badge *>(lv_timer_get_user_data(timer))->refresh(); }
bool Badge::back() { if (editing_) { end_editor(); return true; } return notifyCoreClosed(); }
bool Badge::pause() { touch_active_=false; if (timer_) lv_timer_pause(timer_); stop(); return true; }
bool Badge::resume()
{
    lv_obj_add_flag(image_obj_,LV_OBJ_FLAG_HIDDEN);
    // A resumed worker must not reuse the frame while LVGL is displaying it.
    displayed_revision_=0;
    if (pixels_) requested_.fetch_add(1);
    if (timer_) lv_timer_resume(timer_);
    return start("badge",5120,2);
}
bool Badge::close()
{
    stop();
    if (editing_) { if (!editor_failed_) badge::stop_editor(); editing_=false; }
    if (timer_) { lv_timer_delete(timer_); timer_=nullptr; }
    if (root_) lv_obj_delete(root_);
    root_=image_obj_=welcome_=welcome_text_=add_=message_=controls_=counter_=nullptr;
    previous_=next_=editor_=connection_=qr_=nullptr;
    lv_image_cache_drop(&image_); heap_caps_free(pixels_); pixels_=nullptr;
    return true;
}
}
