/* SPDX-FileCopyrightText: 2026 Waveshare
 * SPDX-License-Identifier: Apache-2.0 */
#include "ButtonTest.hpp"
#include "esp_log.h"
#include "esp_timer.h"
#include <cstdio>
#include <new>

LV_IMAGE_DECLARE(img_app_button_test);

namespace {
constexpr uint32_t GREEN = 0x188563, ERROR = 0xc64f55;
constexpr unsigned SAMPLE_MS = 20, MAX_COUNT = 9999;
} // namespace

using namespace app_ui;
namespace esp_brookesia::apps {
ButtonTest *ButtonTest::instance_;
ButtonTest *ButtonTest::requestInstance()
{
    if (!instance_) instance_ = new (std::nothrow) ButtonTest;
    return instance_;
}
ButtonTest::ButtonTest() : App("Button Test", &img_app_button_test, true, false, false) {}

void ButtonTest::publish(const View &view)
{
    portENTER_CRITICAL(&lock_);
    view_ = view;
    portEXIT_CRITICAL(&lock_);
}
ButtonTest::View ButtonTest::snapshot()
{
    portENTER_CRITICAL(&lock_);
    View view = view_;
    portEXIT_CRITICAL(&lock_);
    return view;
}
ButtonTest::Card ButtonTest::createCard(const char *name, const char *signal, int y)
{
    Card card;
    card.panel = box(root_,12,y,BSP_LCD_H_RES-24,56);
    label(card.panel,name,10,6,78,&lv_font_montserrat_18);
    label(card.panel,signal,10,32,102,&lv_font_montserrat_12,MUTED);
    card.badge=box(card.panel,190,5,94,22,0xf0f3f8,7);
    card.state=label(card.badge,"WAITING",0,0,94,&lv_font_montserrat_12,MUTED);
    lv_obj_set_style_text_align(card.state,LV_TEXT_ALIGN_CENTER,0);
    lv_obj_center(card.state);
    card.count=label(card.panel,"Presses: 0",112,33,94,&lv_font_montserrat_12);
    card.result=label(card.panel,"NOT TESTED",206,33,78,&lv_font_montserrat_12,MUTED);
    lv_obj_set_style_text_align(card.result,LV_TEXT_ALIGN_RIGHT,0);
    return card;
}
bool ButtonTest::run()
{
    root_=screen("Button Test",event,this);
    label(root_,"Press each button to test",12,51,BSP_LCD_H_RES-24,&lv_font_montserrat_12,MUTED);
    const char *names[]={"BOOT","LEFT","RIGHT"};
    const char *pins[]={"GPIO9 / Low","GPIO4 / Low","GPIO5 / Low"};
    for(int i=0;i<3;++i) cards_[i]=createCard(names[i],pins[i],74+i*62);
    summary_=label(root_,"0 / 3 buttons passed",12,264,BSP_LCD_H_RES-24,&lv_font_montserrat_16);
    lv_obj_set_style_text_align(summary_,LV_TEXT_ALIGN_CENTER,0);
    progress_=lv_bar_create(root_);
    lv_obj_set_pos(progress_,28,285); lv_obj_set_size(progress_,BSP_LCD_H_RES-56,4);
    lv_bar_set_range(progress_,0,3);
    button(root_,LV_SYMBOL_REFRESH "  Test again",12,295,BSP_LCD_H_RES-24,22,1,event,this,true);
    publish(View{}); reset_=false; shown_result_=-1;
    refresh();
    timer_=lv_timer_create(poll,20,this);
    if(!timer_ || !start("button_test",4096,2)) { close(); return false; }
    return true;
}

void ButtonTest::work()
{
    View view = snapshot();
    // Read BSP press counters so a press is retained even between UI polls.
    uint32_t previous[3] = {};
    bool baselined[3] = {};
    // Resume keeps completed counts but ignores presses made in another APP.
    for(auto &button:view.buttons) button.state=-1;
    constexpr bsp_button_id_t ids[] = {BSP_BUTTON_ID_BOOT,BSP_BUTTON_ID_LEFT,BSP_BUTTON_ID_RIGHT};
    const char *names[]={"BOOT GPIO9","LEFT GPIO4","RIGHT GPIO5"};
    esp_err_t last_error[] = {ESP_OK,ESP_OK,ESP_OK};
    while (!stopping()) {
        const int64_t sample_start = esp_timer_get_time();
        if (reset_.exchange(false)) {
            view = View{};
            for (auto &item:baselined) item=false;
        }
        for (int i = 0; i < 3; ++i) {
            bsp_button_state_t state = {};
            const esp_err_t ret = bsp_button_get_state(ids[i], &state);
            if (ret != last_error[i]) {
                if (ret == ESP_OK) ESP_LOGI("button_test", "%s input ready", names[i]);
                else ESP_LOGW("button_test", "%s read failed: %s", names[i], esp_err_to_name(ret));
                last_error[i] = ret;
            }
            if (ret != ESP_OK) { view.buttons[i].state = -1; baselined[i] = false; continue; }
            if (baselined[i] && state.press_count != previous[i]) {
                const uint32_t delta = state.press_count - previous[i];
                const uint32_t remaining = MAX_COUNT - view.buttons[i].count;
                view.buttons[i].count += delta < remaining ? delta : remaining;
                ESP_LOGI("button_test", "%s passed, presses=%lu", names[i],
                         static_cast<unsigned long>(view.buttons[i].count));
            }
            if (!baselined[i] || view.buttons[i].state != int(state.pressed))
                ESP_LOGI("button_test", "%s %s", names[i], state.pressed ? "PRESSED" : "RELEASED");
            previous[i] = state.press_count;
            baselined[i] = true;
            view.buttons[i].state = int(state.pressed);
        }
        publish(view);
        const int64_t elapsed_ms = (esp_timer_get_time() - sample_start) / 1000;
        // Always yield; the BSP timer owns debounce and press capture.
        vTaskDelay(pdMS_TO_TICKS(elapsed_ms < SAMPLE_MS ? SAMPLE_MS - elapsed_ms : SAMPLE_MS));
    }
}
void ButtonTest::refresh()
{
    const View view = snapshot();
    int passed = 0;
    bool unavailable = false;
    for (int i = 0; i < 3; ++i) {
        const Reading &reading = view.buttons[i];
        Card &card = cards_[i];
        passed += reading.count > 0;
        unavailable |= reading.state < 0;
        if (card.shown_state == reading.state && card.shown_count == reading.count) continue;
        card.shown_state = reading.state;
        card.shown_count = reading.count;
        const bool pressed = reading.state == 1;
        text(card.state, reading.state < 0 ? "NO INPUT" : pressed ? "PRESSED" : "RELEASED");
        lv_obj_set_style_bg_color(card.badge, lv_color_hex(pressed ? 0xe1f4ec : 0xf0f3f8), 0);
        lv_obj_set_style_text_color(card.state, lv_color_hex(reading.state < 0 ? ERROR : pressed ? GREEN : MUTED), 0);
        lv_obj_set_style_border_color(card.panel, lv_color_hex(pressed ? GREEN : 0xe4eaf2), 0);
        char count[32];
        std::snprintf(count, sizeof(count), "Presses: %lu", static_cast<unsigned long>(reading.count));
        text(card.count, count);
        text(card.result, reading.state < 0 ? "CHECK INPUT" : reading.count ? LV_SYMBOL_OK " PASS" : "NOT TESTED");
        lv_obj_set_style_text_color(card.result, lv_color_hex(reading.state < 0 ? ERROR : reading.count ? GREEN : MUTED), 0);
    }
    const int result = passed | (unavailable ? 8 : 0);
    if (shown_result_ == result) return;
    shown_result_ = result;
    char summary[48];
    std::snprintf(summary,sizeof(summary),"%d / 3 buttons passed",passed);
    text(summary_,summary);
    lv_obj_set_style_text_color(summary_, lv_color_hex(unavailable ? ERROR : passed ? GREEN : INK), 0);

    lv_bar_set_value(progress_, passed, LV_ANIM_OFF);
}
void ButtonTest::event(lv_event_t *event)
{
    auto *app = static_cast<ButtonTest *>(lv_event_get_user_data(event));
    if (action(event) == 0) { app->back(); return; }
    // The worker alone owns the debounce state and counters.
    app->reset_ = true;
}
void ButtonTest::poll(lv_timer_t *timer)
{
    static_cast<ButtonTest *>(lv_timer_get_user_data(timer))->refresh();
}
bool ButtonTest::pause()
{
    if (timer_) lv_timer_pause(timer_);
    stop();
    return true;
}
bool ButtonTest::resume()
{
    if (!start("button_test", 4096, 2)) return false;
    if (timer_) lv_timer_resume(timer_);
    return true;
}
bool ButtonTest::close()
{
    stop();
    if (timer_) { lv_timer_delete(timer_); timer_ = nullptr; }
    if (root_) { lv_obj_delete(root_); root_ = nullptr; }
    for(auto &card:cards_) card=Card{};
    summary_ = hint_ = progress_ = nullptr;
    return true;
}
} // namespace esp_brookesia::apps
