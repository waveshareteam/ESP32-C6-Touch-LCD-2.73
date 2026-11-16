/* SPDX-License-Identifier: Apache-2.0 */
#include "Watch.hpp"
#include "app_support.hpp"
#include <cstdio>
#include <new>

LV_IMAGE_DECLARE(img_app_watch);
namespace {
constexpr uint32_t BG=0xf6f5ef, INK=0x343d3d, MUTED=0x85908a, ACCENT=0x56786a;
constexpr uint8_t SEGMENTS[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
void centered(lv_obj_t *object) { lv_obj_set_style_text_align(object,LV_TEXT_ALIGN_CENTER,0); }
void segment(lv_layer_t *layer,int x,int y,int w,int h)
{
    // Tapered caps give the lit strokes a clean LCD appearance. Unlit strokes
    // are never drawn, leaving the background completely plain.
    const int bevel=LV_MIN(w,h)/2;
    lv_draw_rect_dsc_t core; lv_draw_rect_dsc_init(&core);
    core.bg_color=lv_color_hex(INK);
    lv_draw_triangle_dsc_t cap; lv_draw_triangle_dsc_init(&cap);
    cap.color=core.bg_color;
    if (w>h) {
        const lv_area_t area={x+bevel,y,x+w-bevel-1,y+h-1};
        lv_draw_rect(layer,&core,&area);
        cap.p[0]={x+bevel,y}; cap.p[1]={x,y+bevel}; cap.p[2]={x+bevel,y+h-1};
        lv_draw_triangle(layer,&cap);
        cap.p[0]={x+w-bevel-1,y}; cap.p[1]={x+w-1,y+bevel}; cap.p[2]={x+w-bevel-1,y+h-1};
    } else {
        const lv_area_t area={x,y+bevel,x+w-1,y+h-bevel-1};
        lv_draw_rect(layer,&core,&area);
        cap.p[0]={x,y+bevel}; cap.p[1]={x+bevel,y}; cap.p[2]={x+w-1,y+bevel};
        lv_draw_triangle(layer,&cap);
        cap.p[0]={x,y+h-bevel-1}; cap.p[1]={x+bevel,y+h-1}; cap.p[2]={x+w-1,y+h-bevel-1};
    }
    lv_draw_triangle(layer,&cap);
}
}
namespace esp_brookesia::apps {
Watch *Watch::instance_;
Watch *Watch::requestInstance() { if (!instance_) instance_=new(std::nothrow) Watch; return instance_; }
Watch::Watch():App("Watch",&img_app_watch,true,false,false) {}
Watch::~Watch() { close(); instance_=nullptr; }
bool Watch::run()
{
    valid_=false; last_second_=(time_t)-1; last_source_=WIFI_CLOCK_UNSET;
    for (auto &v:values_) v=-1;
    root_=app_ui::box(lv_screen_active(),0,0,320,320,BG,0);
    auto *back_button=app_ui::button(root_,LV_SYMBOL_LEFT,12,12,32,32,0,event,this);
    lv_obj_set_style_bg_color(back_button,lv_color_hex(0xe9ece4),0);
    lv_obj_set_style_text_color(lv_obj_get_child(back_button,0),lv_color_hex(INK),0);
    app_ui::label(root_,"CLOCK",56,22,130,&lv_font_montserrat_12,MUTED);
    auto *zone=app_ui::label(root_,"UTC+8",224,21,80,&lv_font_montserrat_12,ACCENT);
    lv_obj_set_style_text_align(zone,LV_TEXT_ALIGN_RIGHT,0);
    date_=app_ui::label(root_,"---- / -- / --",12,72,296,&lv_font_montserrat_20,INK);
    weekday_=app_ui::label(root_,"Waiting for RTC",12,101,296,&lv_font_montserrat_12,MUTED);
    centered(date_); centered(weekday_);
    // Draw seven-segment digits directly; no large bitmap or extra frame buffer.
    digits_=app_ui::box(root_,0,136,320,98,BG,0);
    lv_obj_add_event_cb(digits_,draw_digits,LV_EVENT_DRAW_MAIN,this);
    seconds_=app_ui::label(root_,"--",12,251,296,&lv_font_montserrat_20,ACCENT);
    centered(seconds_);
    source_=app_ui::label(root_,"Waiting for time",12,294,296,&lv_font_montserrat_12,MUTED);
    centered(source_);
    refresh();
    timer_=lv_timer_create(tick,200,this);
    if (!timer_) { close(); return false; }
    return true;
}
void Watch::draw_digits(lv_event_t *event)
{
    auto *self=static_cast<Watch *>(lv_event_get_user_data(event));
    auto *layer=lv_event_get_layer(event);
    lv_area_t bounds; lv_obj_get_coords(self->digits_,&bounds);
    constexpr int X[]={20,82,180,242};
    constexpr int SHAPES[7][4]={
        {7,0,38,8},{44,6,8,36},{44,54,8,36},{7,88,38,8},
        {0,54,8,36},{0,6,8,36},{7,44,38,8}};
    for (unsigned digit=0;digit<4;++digit) {
        const uint8_t mask=self->values_[digit]<0?0x40:SEGMENTS[self->values_[digit]];
        for (unsigned bit=0;bit<7;++bit) {
            if (!(mask&(1U<<bit))) continue;
            const auto &s=SHAPES[bit];
            const int x=bounds.x1+X[digit]+s[0], y=bounds.y1+s[1];
            segment(layer,x,y,s[2],s[3]);
        }
    }
    if (self->colon_) {
        lv_draw_rect_dsc_t dot; lv_draw_rect_dsc_init(&dot);
        dot.radius=LV_RADIUS_CIRCLE; dot.bg_color=lv_color_hex(INK);
        for (int y=27;y<=65;y+=38) {
            const lv_area_t area={bounds.x1+155,bounds.y1+y,bounds.x1+164,bounds.y1+y+9};
            lv_draw_rect(layer,&dot,&area);
        }
    }
}
void Watch::refresh()
{
    wifi_clock_snapshot_t clock={};
    const bool valid=wifi_manager_get_clock(&clock);
    if (valid && (!valid_ || clock.instant.tv_sec!=last_second_ || clock.source!=last_source_)) {
        const auto &local=clock.local;
        values_[0]=local.tm_hour/10; values_[1]=local.tm_hour%10;
        values_[2]=local.tm_min/10; values_[3]=local.tm_min%10;
        colon_=(local.tm_sec%2)==0;
        static const char *DAYS[]={"SUNDAY","MONDAY","TUESDAY","WEDNESDAY","THURSDAY","FRIDAY","SATURDAY"};
        char text[48];
        std::snprintf(text,sizeof(text),"%04d / %02d / %02d",local.tm_year+1900,local.tm_mon+1,local.tm_mday);
        app_ui::text(date_,text); app_ui::text(weekday_,DAYS[local.tm_wday]);
        std::snprintf(text,sizeof(text),"%02d",local.tm_sec); app_ui::text(seconds_,text);
        const bool network=clock.source==WIFI_CLOCK_NETWORK;
        app_ui::text(source_,network?"NTP synced":"RTC clock");
        lv_obj_set_style_text_color(source_,lv_color_hex(network?ACCENT:MUTED),0);
        last_second_=clock.instant.tv_sec; last_source_=clock.source;
        lv_obj_invalidate(digits_);
    } else if (!valid && valid_) {
        for (auto &v:values_) v=-1;
        app_ui::text(date_,"---- / -- / --"); app_ui::text(weekday_,"Waiting for RTC");
        app_ui::text(seconds_,"--"); app_ui::text(source_,"Waiting for time");
        lv_obj_set_style_text_color(source_,lv_color_hex(MUTED),0);
        lv_obj_invalidate(digits_);
    }
    valid_=valid;
}
void Watch::event(lv_event_t *event) { static_cast<Watch *>(lv_event_get_user_data(event))->back(); }
void Watch::tick(lv_timer_t *timer) { static_cast<Watch *>(lv_timer_get_user_data(timer))->refresh(); }
bool Watch::pause() { if (timer_) lv_timer_pause(timer_); return true; }
bool Watch::resume() { refresh(); if (timer_) lv_timer_resume(timer_); return true; }
bool Watch::close()
{
    if (timer_) { lv_timer_delete(timer_); timer_=nullptr; }
    if (root_) lv_obj_delete(root_);
    root_=digits_=date_=weekday_=seconds_=source_=nullptr;
    return true;
}
}
