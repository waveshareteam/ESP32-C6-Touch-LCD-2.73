#pragma once

// Private, allocation-free storage/protocol primitives. All callers serialize I/O.
#include "badge_service.hpp"
#include "badge_flash_schedule.hpp"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

namespace badge { namespace detail {
constexpr unsigned HEADER_BYTES = 16;
constexpr unsigned IO_BYTES = 1024;

inline uint32_t crc_update(uint32_t crc, const uint8_t *bytes, size_t count)
{
    while (count--) {
        crc ^= *bytes++;
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1u)));
    }
    return crc;
}
inline void put32(uint8_t *p, uint32_t n)
{
    for (unsigned i = 0; i < 4; ++i) p[i] = uint8_t(n >> (8 * i));
}
inline uint32_t get32(const uint8_t *p)
{
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) |
           (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}
inline void image_header(uint8_t *header, uint32_t crc, bool indexed = false)
{
    memcpy(header, indexed ? "B256" : "B332", 4);
    put32(header + 4, indexed ? PIXEL_BYTES : RAW_BYTES);
    put32(header + 8, crc);
    header[12] = WIDTH & 255; header[13] = WIDTH >> 8;
    header[14] = HEIGHT & 255; header[15] = HEIGHT >> 8;
}
inline bool header_valid(const uint8_t *header)
{
    const bool format = (memcmp(header, "B332", 4) == 0 && get32(header + 4) == RAW_BYTES) ||
                        (memcmp(header, "B256", 4) == 0 && get32(header + 4) == PIXEL_BYTES);
    return format &&
           (header[12] | (unsigned(header[13]) << 8)) == WIDTH &&
           (header[14] | (unsigned(header[15]) << 8)) == HEIGHT;
}
inline bool opaque_palette(const uint8_t *bytes, unsigned offset, unsigned count)
{
    for (unsigned i = 0; i < count && offset + i < PALETTE_BYTES; ++i)
        if (((offset + i) & 3) == 3 && bytes[i] != 255) return false;
    return true;
}
inline void palette(uint8_t *output)
{
    for (unsigned i = 0; i < 256; ++i) {
        output[4 * i] = (i & 3) * 85;
        output[4 * i + 1] = (((i >> 2) & 7) * 255 + 3) / 7;
        output[4 * i + 2] = ((i >> 5) * 255 + 3) / 7;
        output[4 * i + 3] = 255;
    }
}
inline bool slot_value(const char *value, int &slot)
{
    if (!value || value[0] < '0' || value[0] >= '0' + SLOTS || value[1]) return false;
    slot = value[0] - '0';
    return true;
}
inline bool remove_if_present(const char *path)
{
    return unlink(path) == 0 || errno == ENOENT;
}
inline bool exists(const char *path)
{
    struct stat st = {};
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}
inline void suffix(const char *path, const char *ending, char *out, size_t cap)
{
    snprintf(out, cap, "%s%s", path, ending);
}
inline void slot_path(int slot, char *out, size_t cap)
{
    snprintf(out, cap, "/badge/slot_%d.i8", slot);
}
inline bool flush_close(FILE *file)
{
    bool good = fflush(file) == 0;
    if (good && fsync(fileno(file)) != 0) good = false;
    if (fclose(file) != 0) good = false;
    return good;
}

// The file is closed by the caller. Reads into the caller's frame or 1 KiB scratch.
inline bool read_image(FILE *file, uint8_t *pixels = nullptr,
                       Cancel cancel = nullptr, void *context = nullptr)
{
    uint8_t header[HEADER_BYTES], scratch[IO_BYTES];
    if (fread(header, 1, sizeof(header), file) != sizeof(header) || !header_valid(header)) return false;
    const bool indexed = memcmp(header, "B256", 4) == 0;
    const unsigned total = get32(header + 4);
    uint32_t crc = 0xffffffffu;
    FlashSchedule schedule;
    for (unsigned offset = 0; offset < total; offset += IO_BYTES) {
        if (cancel && cancel(context)) return false;
        uint8_t *chunk = pixels ? pixels + (indexed ? 0 : PALETTE_BYTES) + offset : scratch;
        if (fread(chunk, 1, IO_BYTES, file) != IO_BYTES) return false;
        if (indexed && !opaque_palette(chunk, offset, IO_BYTES)) return false;
        crc = crc_update(crc, chunk, IO_BYTES);
        schedule.checkpoint();
    }
    if (fgetc(file) != EOF || ferror(file) || ~crc != get32(header + 8)) return false;
    if (pixels && !indexed) palette(pixels);
    return true;
}
inline int read_active(FILE *file)
{
    uint8_t value[8];
    if (fread(value, 1, sizeof(value), file) != sizeof(value) ||
        fgetc(file) != EOF || ferror(file) || memcmp(value, "A332", 4) ||
        value[4] >= SLOTS || value[5] != uint8_t(~value[4]) || value[6] || value[7]) return -1;
    return value[4];
}
inline bool valid_file(const char *path, bool image)
{
    FILE *file = fopen(path, "rb");
    if (!file) return false;
    bool good = image ? read_image(file) : read_active(file) >= 0;
    if (fclose(file) != 0) good = false;
    return good;
}

// SPIFFS rename cannot replace a destination. Keep the old committed file until
// the new, flushed file has been renamed; recovery always prefers a valid main.
inline bool commit_temp(const char *path)
{
    char temp[64], backup[64];
    suffix(path, ".tmp", temp, sizeof(temp));
    suffix(path, ".bak", backup, sizeof(backup));
    if (!remove_if_present(backup)) return false;
    bool previous = exists(path);
    if (previous && rename(path, backup) != 0) return false;
    if (rename(temp, path) != 0) {
        if (previous) rename(backup, path);
        return false;
    }
    // Cleanup failure cannot undo an already committed replacement.
    remove_if_present(backup);
    return true;
}
inline bool recover(const char *path, bool image)
{
    char temp[64], backup[64], deleted[64];
    suffix(path, ".tmp", temp, sizeof(temp));
    suffix(path, ".bak", backup, sizeof(backup));
    suffix(path, ".del", deleted, sizeof(deleted));
    if (!remove_if_present(temp)) return false;
    // A rename to .del commits deletion. Never resurrect its earlier backup.
    if (exists(deleted)) {
        if (!remove_if_present(backup) || !remove_if_present(deleted)) return false;
    }
    bool main_good = valid_file(path, image);
    bool backup_good = valid_file(backup, image);
    if (!main_good && backup_good) {
        if (!remove_if_present(path) || rename(backup, path) != 0) return false;
        return true;
    }
    if (!main_good && !remove_if_present(path)) return false;
    return remove_if_present(backup);
}
inline bool write_active(const char *path, int slot)
{
    char temp[64]; suffix(path, ".tmp", temp, sizeof(temp));
    FILE *file = fopen(temp, "wb");
    if (!file) return false;
    const uint8_t value[8] = {'A', '3', '3', '2', uint8_t(slot), uint8_t(~slot), 0, 0};
    bool good = fwrite(value, 1, sizeof(value), file) == sizeof(value);
    if (!flush_close(file)) good = false;
    if (good) good = commit_temp(path);
    if (!good) remove_if_present(temp);
    return good;
}
inline bool erase_image(const char *path)
{
    char backup[64], deleted[64];
    suffix(path, ".bak", backup, sizeof(backup));
    suffix(path, ".del", deleted, sizeof(deleted));
    if (!remove_if_present(backup) || !remove_if_present(deleted) || rename(path, deleted) != 0) return false;
    remove_if_present(deleted);
    return true;
}
} }
