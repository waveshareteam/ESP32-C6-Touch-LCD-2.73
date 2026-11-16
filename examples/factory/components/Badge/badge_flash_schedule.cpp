#include "badge_flash_schedule.hpp"
#include "esp_partition.h"
// Use the actual IDF port types, never a copied layout of fs->user_data.
#include "spiffs_api.h"
#include <cstring>

namespace {
void checkpoint_assets(spiffs *fs)
{
    const auto *port = static_cast<const esp_spiffs_t *>(fs->user_data);
    if (!port || !port->partition || strcmp(port->partition->label, "assets")) return;
    // IDF/SPIFFS holds this volume's mutex during mount, format and file I/O.
    // No re-entry or Flash/VFS calls while sleeping. Other volumes are untouched.
    static badge::detail::FlashSchedule schedule;
    schedule.checkpoint();
}
}

extern "C" {
s32_t __real_spiffs_api_read(spiffs *, uint32_t, uint32_t, uint8_t *);
s32_t __real_spiffs_api_write(spiffs *, uint32_t, uint32_t, uint8_t *);
s32_t __real_spiffs_api_erase(spiffs *, uint32_t, uint32_t);

s32_t __wrap_spiffs_api_read(spiffs *fs, uint32_t addr, uint32_t size, uint8_t *dst)
{
    const s32_t result = __real_spiffs_api_read(fs, addr, size, dst);
    // Flash driver has restored cache/interrupts and released its locks here.
    checkpoint_assets(fs);
    return result;
}
s32_t __wrap_spiffs_api_write(spiffs *fs, uint32_t addr, uint32_t size, uint8_t *src)
{
    const s32_t result = __real_spiffs_api_write(fs, addr, size, src);
    checkpoint_assets(fs);
    return result;
}
s32_t __wrap_spiffs_api_erase(spiffs *fs, uint32_t addr, uint32_t size)
{
    const s32_t result = __real_spiffs_api_erase(fs, addr, size);
    checkpoint_assets(fs);
    return result;
}
}
