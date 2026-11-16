#include "badge_service.hpp"
#include "badge_storage.hpp"
#include "wifi_manager.h"
#include "esp_http_server.h"
#include "esp_partition.h"
#include "esp_spiffs.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <atomic>

namespace badge {
namespace {
constexpr const char *TAG = "badge";
constexpr const char *ACTIVE_PATH = "/badge/active";
constexpr int64_t TRANSFER_US = 30000000;
SemaphoreHandle_t mutex = nullptr;
State state;
std::atomic<bool> editing{false}, finished{false};
std::atomic<uint32_t> editor_generation{0};
std::atomic<bool> initializing{false};
enum class InitPhase : uint8_t {
    Idle, Mounting, CheckingBlank, Formatting, Remounting, Recovering, Ready, Failed
};
std::atomic<InitPhase> init_status{InitPhase::Idle};
extern const char html_start[] asm("_binary_badge_html_start");
extern const char html_end[] asm("_binary_badge_html_end");

struct Lock {
    bool held = false;
    explicit Lock(Cancel cancel = nullptr, void *context = nullptr) {
        if (!cancel) { held = xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE; return; }
        TickType_t ticks = pdMS_TO_TICKS(10);
        if (!ticks) ticks = 1;
        while (!cancel(context)) {
            if (xSemaphoreTake(mutex, ticks) == pdTRUE) {
                if (cancel(context)) { xSemaphoreGive(mutex); return; }
                held = true; return;
            }
        }
    }
    ~Lock() { if (held) xSemaphoreGive(mutex); }
    Lock(const Lock &) = delete;
    Lock &operator=(const Lock &) = delete;
};
void message(char *out, unsigned cap, const char *text)
{
    if (out && cap) snprintf(out, cap, "%s", text);
}
void init_stage(InitPhase phase, int slot = -1)
{
    init_status.store(phase);
    const char *names[] = {"idle", "mounting", "checking blank partition", "formatting blank partition",
                           "remounting", "recovering", "ready", "failed"};
    ESP_LOGI(TAG, "Storage init: %s (slot=%d)", names[unsigned(phase)], slot);
}
void state_error(const char *text)
{
    if (!state.ready) init_stage(InitPhase::Failed);
    message(state.error, sizeof(state.error), text);
    ++state.revision;
    ESP_LOGE(TAG, "%s (errno=%d)", text, errno);
}
void clear_error() { state.error[0] = 0; }
int first_slot(uint8_t mask)
{
    for (int slot = 0; slot < SLOTS; ++slot) if (mask & (1u << slot)) return slot;
    return 0;
}
bool select_locked(int slot)
{
    if (!state.ready || slot < 0 || slot >= SLOTS || !(state.available & (1u << slot))) return false;
    // Persist even when unchanged, so a previous metadata failure can be retried.
    if (!detail::write_active(ACTIVE_PATH, slot)) {
        state_error("Could not save active slot. Retry selection.");
        return false;
    }
    state.active = slot;
    clear_error();
    ++state.revision;
    return true;
}

bool partition_erased(const esp_partition_t *partition)
{
    uint8_t buffer[detail::IO_BYTES];
    detail::FlashSchedule schedule;
    for (size_t offset = 0; offset < partition->size; offset += sizeof(buffer)) {
        size_t count = partition->size - offset;
        if (count > sizeof(buffer)) count = sizeof(buffer);
        if (esp_partition_read(partition, offset, buffer, count) != ESP_OK) return false;
        for (size_t i = 0; i < count; ++i) if (buffer[i] != 0xff) return false;
        schedule.checkpoint();
    }
    return true;
}
bool mount_storage()
{
    const auto *partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA,
                                                    ESP_PARTITION_SUBTYPE_DATA_SPIFFS, "assets");
    if (!partition) { state_error("Missing assets SPIFFS partition."); return false; }
    esp_vfs_spiffs_conf_t cfg = {};
    cfg.base_path = "/badge";
    cfg.partition_label = "assets";
    cfg.max_files = 4;
    cfg.format_if_mount_failed = false;
    init_stage(InitPhase::Mounting);
    esp_err_t result = esp_vfs_spiffs_register(&cfg);
    if (result == ESP_FAIL) {
        init_stage(InitPhase::CheckingBlank);
        if (partition_erased(partition)) {
            // Formatting an unmounted partition is supported by ESP-IDF. Never
            // automatically format a nonblank/corrupted or already mounted volume.
            init_stage(InitPhase::Formatting);
            result = esp_spiffs_format("assets");
            if (result == ESP_OK) {
                init_stage(InitPhase::Remounting);
                result = esp_vfs_spiffs_register(&cfg);
            }
        }
    }
    if (result != ESP_OK) {
        char text[96];
        snprintf(text, sizeof(text), "Badge storage: %s. Nonblank assets will not be erased.", esp_err_to_name(result));
        state_error(text);
        return false;
    }
    return true;
}

// Close error responses explicitly: a rejected request may still have unread
// body bytes, which must never be interpreted as the next keep-alive request.
esp_err_t fail(httpd_req_t *req, httpd_err_code_t code, const char *text)
{
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_send_err(req, code, text);
    return ESP_FAIL;
}
bool allowed(httpd_req_t *req)
{
    wifi_manager_touch_web_session();
    if (editing.load() && wifi_manager_web_transfer_active()) return true;
    fail(req, HTTPD_403_FORBIDDEN, "Badge editor is closed. Reopen /badge.");
    return false;
}
bool request_slot(httpd_req_t *req, int &slot)
{
    char query[96], value[8];
    return httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK &&
           httpd_query_key_value(query, "slot", value, sizeof(value)) == ESP_OK &&
           detail::slot_value(value, slot);
}
esp_err_t ok(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
}
bool no_body(httpd_req_t *req)
{
    if (!req->content_len) return true;
    fail(req, HTTPD_400_BAD_REQUEST, "This operation takes no request body.");
    return false;
}
esp_err_t index_get(httpd_req_t *req)
{
    if (!wifi_manager_begin_web_transfer()) {
        httpd_resp_set_status(req, "409 Conflict");
        httpd_resp_set_type(req, "text/html; charset=utf-8");
        return httpd_resp_sendstr(req, "<meta name='viewport' content='width=device-width,initial-scale=1'>"
            "<p>Wait for Wi-Fi setup or scanning to finish, then retry.</p><a href='/'>Device home</a>");
    }
    // Mount on the HTTP task only if the APP worker has not initialized yet.
    if (!initialize()) {
        wifi_manager_end_web_transfer();
        return fail(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Badge storage unavailable. Check the device and retry.");
    }
    ++editor_generation;
    finished = false;
    editing = true;
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    return httpd_resp_send(req, html_start, html_end - html_start);
}
esp_err_t state_get(httpd_req_t *req)
{
    if (!allowed(req)) return ESP_FAIL;
    State copy;
    { Lock lock; copy = state; }
    // Fixed-size JSON avoids a heap allocation for each slot or state poll.
    char json[1024];
    int used = snprintf(json, sizeof(json),
        "{\"ready\":%s,\"width\":%d,\"height\":%d,\"format\":\"indexed8\",\"uploadBytes\":%u,"
        "\"slotCount\":%d,\"activeSlot\":%d,\"revision\":%lu,\"slots\":[",
        copy.ready ? "true" : "false", WIDTH, HEIGHT, PIXEL_BYTES, SLOTS, copy.active,
        static_cast<unsigned long>(copy.revision));
    for (int slot = 0; slot < SLOTS; ++slot) {
        if (used < 0 || size_t(used) >= sizeof(json)) break;
        const bool present = (copy.available & (1u << slot)) != 0;
        int count = snprintf(json + used, sizeof(json) - used,
            "%s{\"slot\":%d,\"hasImage\":%s,\"isActive\":%s}", slot ? "," : "", slot,
            present ? "true" : "false", present && copy.active == slot ? "true" : "false");
        if (count < 0) return fail(req, HTTPD_500_INTERNAL_SERVER_ERROR, "State encoding failed.");
        used += count;
    }
    if (used < 0 || size_t(used) + 3 > sizeof(json))
        return fail(req, HTTPD_500_INTERNAL_SERVER_ERROR, "State encoding failed.");
    memcpy(json + used, "]}", 3);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    return httpd_resp_sendstr(req, json);
}
esp_err_t image_get(httpd_req_t *req)
{
    if (!allowed(req)) return ESP_FAIL;
    int slot;
    if (!request_slot(req, slot)) return fail(req, HTTPD_400_BAD_REQUEST, "Invalid slot (use 0 through 5).");
    Lock lock;
    if (!state.ready || !(state.available & (1u << slot)))
        return fail(req, HTTPD_404_NOT_FOUND, "Image unavailable.");
    char path[48]; detail::slot_path(slot, path, sizeof(path));
    FILE *file = fopen(path, "rb");
    if (!file) return fail(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Cannot read image.");
    // Validate before sending. Browser and LVGL receive the same BGRA palette
    // and indices; old RGB332 files get their original palette prepended.
    bool good = detail::read_image(file) && fseek(file, 0, SEEK_SET) == 0;
    if (!good) {
        fclose(file);
        return fail(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Stored image checksum failed.");
    }
    httpd_resp_set_type(req, "application/octet-stream");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    uint8_t buffer[detail::IO_BYTES];
    uint8_t header[detail::HEADER_BYTES];
    if (fread(header, 1, sizeof(header), file) != sizeof(header)) {
        fclose(file);
        return ESP_FAIL;
    }
    const bool indexed = memcmp(header, "B256", 4) == 0;
    const unsigned total = indexed ? PIXEL_BYTES : RAW_BYTES;
    const int64_t deadline = esp_timer_get_time() + TRANSFER_US;
    esp_err_t result = ESP_OK;
    if (!indexed) {
        detail::palette(buffer);
        result = httpd_resp_send_chunk(req, reinterpret_cast<const char *>(buffer), PALETTE_BYTES);
    }
    detail::FlashSchedule schedule;
    for (unsigned sent = 0; result == ESP_OK && sent < total; sent += sizeof(buffer)) {
        if (!editing.load() || esp_timer_get_time() >= deadline ||
            fread(buffer, 1, sizeof(buffer), file) != sizeof(buffer)) { result = ESP_FAIL; break; }
        result = httpd_resp_send_chunk(req, reinterpret_cast<const char *>(buffer), sizeof(buffer));
        if (result != ESP_OK) break;
        wifi_manager_touch_web_session();
        schedule.checkpoint();
    }
    if (fclose(file) != 0) result = ESP_FAIL;
    return result == ESP_OK ? httpd_resp_send_chunk(req, nullptr, 0) : ESP_FAIL;
}
esp_err_t upload_post(httpd_req_t *req)
{
    if (!allowed(req)) return ESP_FAIL;
    int slot;
    if (!request_slot(req, slot)) return fail(req, HTTPD_400_BAD_REQUEST, "Invalid slot (use 0 through 5).");
    const bool indexed = req->content_len == PIXEL_BYTES;
    if (!indexed && req->content_len != RAW_BYTES)
        return fail(req, HTTPD_400_BAD_REQUEST, "Indexed upload requires 103424 bytes (legacy RGB332: 102400).");
    const unsigned total = indexed ? PIXEL_BYTES : RAW_BYTES;
    char type[40];
    if (httpd_req_get_hdr_value_str(req, "Content-Type", type, sizeof(type)) != ESP_OK ||
        strcmp(type, "application/octet-stream"))
        return fail(req, HTTPD_400_BAD_REQUEST, "Use Content-Type: application/octet-stream.");

    const uint32_t generation = editor_generation.load();
    Lock lock;
    if (!state.ready) return fail(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Storage is not ready.");
    char path[48], temp[64];
    detail::slot_path(slot, path, sizeof(path));
    detail::suffix(path, ".tmp", temp, sizeof(temp));
    FILE *file = fopen(temp, "wb");
    if (!file) {
        state_error("Could not open upload file.");
        return fail(req, HTTPD_500_INTERNAL_SERVER_ERROR, state.error);
    }
    uint8_t buffer[detail::IO_BYTES], header[detail::HEADER_BYTES] = {};
    bool storage_ok = fwrite(header, 1, sizeof(header), file) == sizeof(header);
    bool palette_ok = true;
    unsigned received = 0;
    uint32_t crc = 0xffffffffu;
    const int64_t deadline = esp_timer_get_time() + TRANSFER_US;
    detail::FlashSchedule schedule;
    while (storage_ok && received < total && editing.load() &&
           editor_generation.load() == generation && esp_timer_get_time() < deadline) {
        schedule.checkpoint();
        unsigned count = total - received;
        if (count > sizeof(buffer)) count = sizeof(buffer);
        const int n = httpd_req_recv(req, reinterpret_cast<char *>(buffer), count);
        if (n == HTTPD_SOCK_ERR_TIMEOUT) continue;
        if (n <= 0 || unsigned(n) > count) break;
        if (indexed && !detail::opaque_palette(buffer, received, n)) { palette_ok = false; break; }
        crc = detail::crc_update(crc, buffer, n);
        storage_ok = fwrite(buffer, 1, n, file) == unsigned(n);
        received += n;
        wifi_manager_touch_web_session();
    }
    bool complete = received == total && editing.load() && editor_generation.load() == generation &&
                    esp_timer_get_time() < deadline;
    if (storage_ok && complete) {
        detail::image_header(header, ~crc, indexed);
        storage_ok = fseek(file, 0, SEEK_SET) == 0 && fwrite(header, 1, sizeof(header), file) == sizeof(header);
    }
    if (!detail::flush_close(file)) storage_ok = false;
    bool saved = storage_ok && complete && editing.load() && editor_generation.load() == generation &&
                 detail::commit_temp(path);
    if (!saved) {
        detail::remove_if_present(temp);
        if (!palette_ok)
            return fail(req, HTTPD_400_BAD_REQUEST, "Palette must contain 256 opaque BGRA colors. Previous image retained.");
        if (!storage_ok || complete) {
            state_error("Could not commit image. Previous image retained; retry upload.");
            return fail(req, HTTPD_500_INTERNAL_SERVER_ERROR, state.error);
        }
        return fail(req, HTTPD_400_BAD_REQUEST, "Upload interrupted, expired or editor closed. Previous image retained.");
    }
    // No slot/revision is published until the complete file is committed.
    state.available |= uint8_t(1u << slot);
    ++state.revision;
    if (!select_locked(slot))
        return fail(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Image saved, but selection failed. Refresh and select again.");
    return ok(req);
}
esp_err_t active_post(httpd_req_t *req)
{
    if (!allowed(req) || !no_body(req)) return ESP_FAIL;
    int slot;
    if (!request_slot(req, slot)) return fail(req, HTTPD_400_BAD_REQUEST, "Invalid slot.");
    Lock lock;
    if (!state.ready || !(state.available & (1u << slot))) return fail(req, HTTPD_400_BAD_REQUEST, "Slot is empty.");
    return select_locked(slot) ? ok(req) : fail(req, HTTPD_500_INTERNAL_SERVER_ERROR, state.error);
}
esp_err_t image_delete(httpd_req_t *req)
{
    if (!allowed(req) || !no_body(req)) return ESP_FAIL;
    int slot;
    if (!request_slot(req, slot)) return fail(req, HTTPD_400_BAD_REQUEST, "Invalid slot.");
    Lock lock;
    if (!state.ready || !(state.available & (1u << slot))) return fail(req, HTTPD_404_NOT_FOUND, "Slot is empty.");
    char path[48]; detail::slot_path(slot, path, sizeof(path));
    if (!detail::erase_image(path)) {
        state_error("Could not delete image. Retry deletion.");
        return fail(req, HTTPD_500_INTERNAL_SERVER_ERROR, state.error);
    }
    state.available &= uint8_t(~(1u << slot));
    clear_error();
    ++state.revision;
    if (state.active == slot) {
        // A stale active record is harmless: boot validates it against available.
        state.active = first_slot(state.available);
        if (state.available && !detail::write_active(ACTIVE_PATH, state.active)) {
            state_error("Image deleted, but selection could not be saved. Retry selection.");
            return fail(req, HTTPD_500_INTERNAL_SERVER_ERROR, state.error);
        }
    }
    return ok(req);
}
esp_err_t close_post(httpd_req_t *req)
{
    if (!allowed(req) || !no_body(req)) return ESP_FAIL;
    editing = false;
    ++editor_generation;
    finished = true;
    wifi_manager_end_web_transfer();
    return ok(req);
}
void stop_web_editor()
{
    editing = false;
    ++editor_generation;
    finished = true;
}
esp_err_t register_routes(httpd_handle_t server)
{
    struct Route { const char *uri; httpd_method_t method; esp_err_t (*handler)(httpd_req_t *); };
    const Route routes[] = {
        {"/badge", HTTP_GET, index_get}, {"/badge/", HTTP_GET, index_get},
        {"/badge/state", HTTP_GET, state_get}, {"/badge/image", HTTP_GET, image_get},
        {"/badge/image", HTTP_DELETE, image_delete}, {"/badge/active", HTTP_POST, active_post},
        {"/badge/upload", HTTP_POST, upload_post}, {"/badge/close", HTTP_POST, close_post}
    };
    for (const auto &route : routes) {
        httpd_uri_t uri = {};
        uri.uri = route.uri; uri.method = route.method; uri.handler = route.handler;
        esp_err_t result = httpd_register_uri_handler(server, &uri);
        if (result != ESP_OK) return result;
    }
    return ESP_OK;
}
}

bool prepare()
{
    // Main calls this before Wi-Fi startup, before worker/HTTP concurrency.
    if (!mutex) mutex = xSemaphoreCreateMutex();
    return mutex && wifi_manager_register_local_portal(register_routes, stop_web_editor);
}
bool initialize(Cancel cancel, void *context)
{
    if (!mutex) return false;
    Lock lock(cancel, context);
    if (!lock.held) return false;
    if (state.ready) return true;
    // Keep a successful mount when retrying recovery after an I/O failure.
    static bool mounted = false;
    if (!mounted) { if (!mount_storage()) return false; mounted = true; }
    init_stage(InitPhase::Recovering);
    if (!detail::recover(ACTIVE_PATH, false)) { state_error("Could not recover active slot."); return false; }
    uint8_t available = 0;
    for (int slot = 0; slot < SLOTS; ++slot) {
        init_stage(InitPhase::Recovering, slot);
        char path[48]; detail::slot_path(slot, path, sizeof(path));
        if (!detail::recover(path, true)) { state_error("Could not recover badge images."); return false; }
        if (detail::exists(path)) available |= uint8_t(1u << slot);
    }
    int active = -1;
    FILE *file = fopen(ACTIVE_PATH, "rb");
    if (file) { active = detail::read_active(file); fclose(file); }
    if (active < 0 || !(available & (1u << active))) active = first_slot(available);
    state.available = available;
    state.active = active;
    clear_error();
    state.ready = true;
    init_stage(InitPhase::Ready);
    ++state.revision;
    ESP_LOGI(TAG, "Six 320x320 indexed-color slots ready on assets SPIFFS");
    return true;
}
bool request_initialize()
{
    if (!mutex) return false;
    State copy;
    if (snapshot(copy) && copy.ready) return true;
    bool expected = false;
    if (!initializing.compare_exchange_strong(expected, true)) return true;
    auto worker = [](void *) {
        initialize();
        initializing = false;
        vTaskDelete(nullptr);
    };
    if (xTaskCreate(worker, "badge_init", 4096, nullptr, 2, nullptr) == pdPASS) return true;
    initializing = false;
    // Never wait behind a browser transfer on the GUI task.
    if (xSemaphoreTake(mutex, 0) == pdTRUE) {
        state_error("Not enough memory to start badge storage initialization.");
        xSemaphoreGive(mutex);
    }
    return false;
}
bool snapshot(State &out)
{
    if (!mutex || xSemaphoreTake(mutex, 0) != pdTRUE) return false;
    out = state;
    xSemaphoreGive(mutex);
    return true;
}
const char *initialization_status()
{
    switch (init_status.load()) {
    case InitPhase::Mounting:
    case InitPhase::Remounting: return "Opening storage";
    case InitPhase::CheckingBlank: return "Checking storage";
    case InitPhase::Formatting: return "Preparing storage";
    case InitPhase::Recovering: return "Loading images";
    default: return nullptr;
    }
}
bool load(int slot, uint8_t *pixels, char *error, unsigned capacity, Cancel cancel, void *context)
{
    message(error, capacity, "");
    if (!mutex || !pixels || slot < 0 || slot >= SLOTS) {
        message(error, capacity, "Invalid badge image request."); return false;
    }
    Lock lock(cancel, context);
    if (!lock.held) { message(error, capacity, "Badge load cancelled."); return false; }
    if (!state.ready || !(state.available & (1u << slot))) {
        message(error, capacity, "Badge image unavailable."); return false;
    }
    char path[48]; detail::slot_path(slot, path, sizeof(path));
    FILE *file = fopen(path, "rb");
    bool good = file && detail::read_image(file, pixels, cancel, context);
    if (file && fclose(file) != 0) good = false;
    if (!good) message(error, capacity, "Cannot read badge image (storage/checksum error).");
    return good;
}
bool select(int slot)
{
    if (!mutex) return false;
    Lock lock;
    return select_locked(slot);
}
bool next(int direction, Cancel cancel, void *context)
{
    if (!mutex || !direction) return false;
    Lock lock(cancel, context);
    if (!lock.held) return false;
    if (!state.ready) return false;
    for (int step = 1; step <= SLOTS; ++step) {
        int candidate = (state.active + (direction > 0 ? step : -step) + SLOTS) % SLOTS;
        if (state.available & (1u << candidate)) return select_locked(candidate);
    }
    return false;
}
bool start_editor()
{
    if (!mutex || !wifi_manager_start_local_portal(register_routes)) return false;
    finished = false;
    editing = true;
    return true;
}
void stop_editor()
{
    // Browser ownership survives leaving the device overlay, as in the reference.
    if (!wifi_manager_web_transfer_active()) { editing = false; ++editor_generation; }
    wifi_manager_stop_local_portal();
}
bool editor_finished() { return finished.exchange(false); }
}
