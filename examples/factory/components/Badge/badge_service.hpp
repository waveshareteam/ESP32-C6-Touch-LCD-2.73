#pragma once

#include <cstdint>

namespace badge {
constexpr int WIDTH = 320, HEIGHT = 320, SLOTS = 6;
constexpr unsigned PALETTE_BYTES = 256 * 4;
constexpr unsigned RAW_BYTES = WIDTH * HEIGHT;
constexpr unsigned PIXEL_BYTES = PALETTE_BYTES + RAW_BYTES;
constexpr unsigned IMAGE_BYTES = PIXEL_BYTES;

struct State {
    bool ready = false;
    uint8_t available = 0;
    int active = 0;
    uint32_t revision = 0;
    char error[96] = {};
};
using Cancel = bool (*)(void *context);

// GUI-safe atomic read, including while initialize() holds the storage mutex.
// Static phase string during initialization; nullptr when inactive (also after
// success/failure). Use snapshot() for ready/error. No percentage is implied.
const char *initialization_status();

// Call once before wifi_manager_init(). Registers routes; does no Flash I/O.
bool prepare();
// Synchronous mount/recovery. Call on the APP worker, never on the GUI task.
// The HTTP task may also call this lazily when opening /badge.
bool initialize(Cancel cancel = nullptr, void *context = nullptr);
// GUI-safe, idempotent request. Independent task owns initialization and never
// retains an APP pointer. Poll snapshot() for ready/error; no join is required.
bool request_initialize();
// Zero-wait copy. On contention returns false and leaves out unchanged.
bool snapshot(State &out);
// Worker only. Caller supplies PIXEL_BYTES and hides the image while writing.
// Writes the photo's 256 opaque BGRA colors, then RAW_BYTES palette indices.
// Legacy RGB332 files are expanded into the same I8 frame layout on read.
// No allocation of another frame and no GUI calls. Discard output on failure.
bool load(int slot, uint8_t *pixels, char *error, unsigned capacity,
          Cancel cancel = nullptr, void *context = nullptr);
// Flash I/O: invoke select/next on a worker, never on the GUI task.
bool select(int slot);
bool next(int direction, Cancel cancel = nullptr, void *context = nullptr);
// GUI-safe portal requests; neither mounts storage nor reads/writes Flash.
bool start_editor();
void stop_editor();
bool editor_finished();
}
