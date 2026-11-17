/* SPDX-License-Identifier: Apache-2.0 */
#include "LegacyApps.hpp"
#include "app_touch_paint.h"
#include "app_pinout.h"
#include <new>
LV_IMAGE_DECLARE(image_draw);
LV_IMAGE_DECLARE(PinsTest);
namespace {
esp_brookesia::systems::phone::App::Config drawing_config() {
    auto c=esp_brookesia::systems::phone::App::Config::SIMPLE_CONSTRUCTOR(&image_draw,false,false);
    c.flags.enable_navigation_gesture=false;
    return c;
}
}
namespace esp_brookesia::apps {
TouchTest *TouchTest::requestInstance() { static auto *p=new(std::nothrow) TouchTest; return p; }
TouchTest::TouchTest():App(systems::base::App::Config::SIMPLE_CONSTRUCTOR("Touch Test",&image_draw,true),drawing_config()) {}
bool TouchTest::run() { return app_touch_paint_open(lv_screen_active(),exit,this); }
void TouchTest::exit(void *p) { static_cast<TouchTest *>(p)->back(); }
bool TouchTest::close() { app_touch_paint_close(); return true; }
bool TouchTest::pause() { app_touch_paint_pause(); return true; }
bool TouchTest::resume() { app_touch_paint_resume(); return true; }
PinsTest *PinsTest::requestInstance() { static auto *p=new(std::nothrow) PinsTest; return p; }
PinsTest::PinsTest():App("PinsTest",&::PinsTest,true,false,false) {}
bool PinsTest::run() { return app_pinout_open(lv_screen_active()); }
bool PinsTest::close() { app_pinout_close(); return true; }
}
