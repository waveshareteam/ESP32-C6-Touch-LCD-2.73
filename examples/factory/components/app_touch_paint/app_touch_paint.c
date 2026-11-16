#include "app_touch_paint.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_lv_adapter_input.h"
#include "lvgl.h"

#define CANVAS_WIDTH 320
#define CANVAS_HEIGHT 320
#define PALETTE_BYTES (16U * sizeof(lv_color32_t))
#define CONTROL_COUNT 8

enum { COLOR_BG, COLOR_BLACK, COLOR_RED, COLOR_BLUE, COLOR_GREEN, COLOR_ORANGE };

static lv_obj_t *s_parent, *s_root;
static void (*s_back)(void *);
static void *s_context;
static lv_obj_t *s_canvas;
static void *s_buffer;
static uint32_t s_stride;
static lv_timer_t *s_touch_timer;
static lv_obj_t *s_controls[CONTROL_COUNT];
static size_t s_control_count;
static lv_point_t s_last;
static uint8_t s_color;
static uint8_t s_width;
static bool s_drawing;
static bool s_multitouch;
static bool s_fullscreen;

LV_IMAGE_DECLARE(image_draw);

static uint8_t *pixels(void)
{
    return (uint8_t *)s_buffer + PALETTE_BYTES;
}

static void invalidate_area(const lv_area_t *area)
{
    if (!s_canvas || !area || area->x2 < area->x1 || area->y2 < area->y1) return;
    lv_draw_buf_flush_cache(lv_canvas_get_draw_buf(s_canvas), area);
    lv_area_t pos;
    lv_obj_get_coords(s_canvas, &pos);
    lv_area_t screen = {pos.x1 + area->x1, pos.y1 + area->y1,
                        pos.x1 + area->x2, pos.y1 + area->y2};
    lv_obj_invalidate_area(s_canvas, &screen);
}

static void clear_canvas(void)
{
    s_drawing = false;
    s_multitouch = false;
    if (!s_buffer) return;
    memset(pixels(), 0, (size_t)s_stride * CANVAS_HEIGHT);
    lv_area_t full = {0, 0, CANVAS_WIDTH - 1, CANVAS_HEIGHT - 1};
    invalidate_area(&full);
}

static void delete_cb(lv_event_t *event)
{
    (void)event;
    heap_caps_free(s_buffer);
    s_buffer = NULL;
    s_canvas = NULL;
}

static bool event_point(lv_event_t *event, lv_point_t *point)
{
    lv_indev_t *indev = lv_event_get_indev(event);
    if (!indev || !s_canvas) return false;
    lv_point_t absolute;
    lv_area_t area;
    lv_indev_get_point(indev, &absolute);
    lv_obj_get_coords(s_canvas, &area);
    point->x = absolute.x - area.x1;
    point->y = absolute.y - area.y1;
    return point->x >= 0 && point->x < CANVAS_WIDTH &&
           point->y >= 0 && point->y < CANVAS_HEIGHT;
}

static void include_point(lv_area_t *area, int32_t x, int32_t y)
{
    if (x < area->x1) area->x1 = x;
    if (y < area->y1) area->y1 = y;
    if (x > area->x2) area->x2 = x;
    if (y > area->y2) area->y2 = y;
}

static void set_pixel(int32_t x, int32_t y)
{
    uint8_t *p = pixels() + (size_t)y * s_stride + (x >> 1);
    if ((x & 1) == 0) *p = (uint8_t)((*p & 0x0F) | (s_color << 4));
    else *p = (uint8_t)((*p & 0xF0) | s_color);
}

static void stamp(const lv_point_t *point, lv_area_t *dirty)
{
    int32_t radius = (s_width + 1) / 2;
    for (int32_t y = -radius; y <= radius; ++y) {
        for (int32_t x = -radius; x <= radius; ++x) {
            if (x * x + y * y > radius * radius) continue;
            int32_t px = point->x + x, py = point->y + y;
            if (px < 0 || px >= CANVAS_WIDTH || py < 0 || py >= CANVAS_HEIGHT) continue;
            set_pixel(px, py);
            include_point(dirty, px, py);
        }
    }
}

static void draw_line(const lv_point_t *from, const lv_point_t *to)
{
    int32_t dx = to->x - from->x, dy = to->y - from->y;
    uint32_t steps = LV_MAX(LV_ABS(dx), LV_ABS(dy));
    if (!steps) steps = 1;
    lv_area_t dirty = {CANVAS_WIDTH, CANVAS_HEIGHT, -1, -1};
    for (uint32_t i = 0; i <= steps; ++i) {
        lv_point_t p = {(lv_coord_t)(from->x + (int64_t)dx * i / steps),
                        (lv_coord_t)(from->y + (int64_t)dy * i / steps)};
        stamp(&p, &dirty);
    }
    invalidate_area(&dirty);
}

static void canvas_cb(lv_event_t *event)
{
    lv_indev_t *indev = lv_event_get_indev(event);
    if (!indev) return;
    uint8_t count = esp_lv_adapter_touch_get_count(indev);
    if (count > 1) {
        s_drawing = false;
        s_multitouch = true;
        return;
    }
    if (s_multitouch) {
        if (count == 0) s_multitouch = false;
        return;
    }

    lv_point_t point;
    lv_event_code_t code = lv_event_get_code(event);
    if (code == LV_EVENT_PRESSED) {
        s_drawing = event_point(event, &point);
        if (s_drawing) { s_last = point; draw_line(&point, &point); }
    } else if (code == LV_EVENT_PRESSING && s_drawing && event_point(event, &point)) {
        draw_line(&s_last, &point);
        s_last = point;
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        s_drawing = false;
    }
}

static void color_cb(lv_event_t *e) { if (lv_event_get_code(e) == LV_EVENT_CLICKED) s_color = (uint8_t)(uintptr_t)lv_event_get_user_data(e); }
static void width_cb(lv_event_t *e) { if (lv_event_get_code(e) == LV_EVENT_CLICKED) s_width = (uint8_t)(uintptr_t)lv_event_get_user_data(e); }

static void remember(lv_obj_t *obj) { if (s_control_count < CONTROL_COUNT) s_controls[s_control_count++] = obj; }

static void fullscreen_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (s_fullscreen) {
        ESP_LOGI("touch_test", "Exit button: returning to desktop");
        if (s_back) s_back(s_context);
        return;
    }
    s_fullscreen = true;
    s_drawing = false;
    for (size_t i = 0; i < s_control_count; ++i) {
        lv_obj_add_flag(s_controls[i], LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_t *button = lv_event_get_current_target(e);
    lv_label_set_text(lv_obj_get_child(button, 0), "Exit");
}

static lv_obj_t *text_button(lv_obj_t *parent, const char *text, int32_t x, int32_t y,
                             int32_t width, lv_event_cb_t cb, void *data)
{
    lv_obj_t *button = lv_button_create(parent);
    lv_obj_set_pos(button, x, y); lv_obj_set_size(button, width, 30);
    lv_obj_set_style_radius(button, 7, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x202C35), 0);
    lv_obj_set_style_shadow_width(button, 0, 0);
    lv_obj_add_event_cb(button, cb, LV_EVENT_CLICKED, data);
    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, text); lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_center(label);
    return button;
}

static lv_obj_t *color_button(lv_obj_t *parent, uint32_t color, uint8_t index, int32_t x)
{
    lv_obj_t *button = lv_button_create(parent);
    lv_obj_set_pos(button, x, 284); lv_obj_set_size(button, 24, 24);
    lv_obj_set_style_radius(button, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(color), 0);
    lv_obj_set_style_border_width(button, 2, 0);
    lv_obj_set_style_border_color(button, lv_color_hex(0xD7E0E7), 0);
    lv_obj_set_style_shadow_width(button, 0, 0);
    lv_obj_add_event_cb(button, color_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)index);
    return button;
}

static lv_obj_t *create_ui(void)
{
    lv_obj_t *screen = lv_obj_create(s_parent);
    lv_obj_remove_style_all(screen);
    lv_obj_set_size(screen, CANVAS_WIDTH, CANVAS_HEIGHT);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x10171D), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(screen, 0, 0);

    s_stride = lv_draw_buf_width_to_stride(CANVAS_WIDTH, LV_COLOR_FORMAT_I4);
    size_t size = (size_t)s_stride * CANVAS_HEIGHT + PALETTE_BYTES;
    s_buffer = heap_caps_aligned_alloc(LV_DRAW_BUF_ALIGN, size, MALLOC_CAP_8BIT);
    if (!s_buffer) {
        ESP_LOGE("Drawing", "Unable to allocate %u-byte canvas", (unsigned)size);
        lv_obj_t *label = lv_label_create(screen);
        lv_label_set_text(label, "Canvas memory unavailable"); lv_obj_center(label);
        return screen;
    }

    s_canvas = lv_canvas_create(screen);
    lv_canvas_set_buffer(s_canvas, s_buffer, CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_FORMAT_I4);
    const uint32_t colors[] = {0xFAFBFC, 0x111820, 0xEF4444, 0x3B82F6, 0x22C55E, 0xF59E0B};
    for (uint8_t i = 0; i < sizeof(colors) / sizeof(colors[0]); ++i)
        lv_canvas_set_palette(s_canvas, i, lv_color_to_32(lv_color_hex(colors[i]), LV_OPA_COVER));
    lv_obj_set_pos(s_canvas, 0, 0); lv_obj_set_size(s_canvas, CANVAS_WIDTH, CANVAS_HEIGHT);
    lv_obj_move_to_index(s_canvas, 0);
    lv_obj_add_flag(s_canvas, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(s_canvas, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(s_canvas, canvas_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(s_canvas, delete_cb, LV_EVENT_DELETE, NULL);
    clear_canvas();

    remember(color_button(screen, 0x111820, COLOR_BLACK, 12));
    remember(color_button(screen, 0xEF4444, COLOR_RED, 43));
    remember(color_button(screen, 0x3B82F6, COLOR_BLUE, 74));
    remember(color_button(screen, 0x22C55E, COLOR_GREEN, 105));
    remember(color_button(screen, 0xF59E0B, COLOR_ORANGE, 136));
    remember(text_button(screen, "2", 190, 281, 34, width_cb, (void *)2));
    remember(text_button(screen, "5", 230, 281, 34, width_cb, (void *)5));
    remember(text_button(screen, "9", 270, 281, 34, width_cb, (void *)9));
    lv_obj_t *toggle = text_button(screen, "UI", 135, 7, 50, fullscreen_cb, NULL);
    lv_obj_set_size(toggle, 50, 30); lv_obj_set_style_bg_opa(toggle, 190, 0);
    return screen;
}

static void app_resume(void) {}
static void app_pause(void) { s_drawing = false; s_multitouch = false; }

static void app_run(void)
{
    s_canvas = NULL; s_buffer = NULL; s_control_count = 0;
    s_color = COLOR_BLACK; s_width = 5; s_drawing = false; s_multitouch = false; s_fullscreen = false;
    lv_indev_t *indev = lv_indev_get_next(NULL);
    while (indev && lv_indev_get_type(indev) != LV_INDEV_TYPE_POINTER) indev = lv_indev_get_next(indev);
    s_touch_timer = indev ? lv_indev_get_read_timer(indev) : NULL;
    if (s_touch_timer) lv_timer_set_period(s_touch_timer, 10);
    s_root = create_ui();
}

static void app_close(void)
{
    s_drawing = false; s_multitouch = false;
    if (s_touch_timer) { lv_timer_set_period(s_touch_timer, LV_DEF_REFR_PERIOD); s_touch_timer = NULL; }
}

bool app_touch_paint_open(lv_obj_t *parent, void (*back)(void *), void *context)
{
    s_parent=parent; s_back=back; s_context=context;
    app_run();
    if (!s_canvas) { app_touch_paint_close(); return false; }
    return true;
}
void app_touch_paint_close(void)
{
    app_close();
    if (s_root) { lv_obj_delete(s_root); s_root=NULL; }
    s_parent=NULL; s_back=NULL; s_context=NULL;
}
void app_touch_paint_pause(void) { app_pause(); }
void app_touch_paint_resume(void) { app_resume(); }
