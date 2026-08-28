// FILENAME: hal_freestanding.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (alloc.h, alloc.c, hal.h, memops.h, memops.c)
// Default freestanding Hardware Abstraction Layer implementation with static arena.
//
// ---- Includes ----

#include "hal/hal.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"

static HalContext global_hal = {0};

HalContext *hal_get(void) {
    return &global_hal;
}

void hal_set(const HalContext *ctx) {
    if (ctx) {
        global_hal = *ctx;
    } else {
        runtime_memset(&global_hal, 0, sizeof(global_hal));
    }
}

// Memory HAL fallback using built-in pool allocator
static void *fs_alloc(size_t size) {
    return runtime_malloc(size);
}

static void *fs_calloc(size_t count, size_t size) {
    return runtime_calloc(count, size);
}

static void *fs_realloc(void *ptr, size_t size) {
    return runtime_realloc(ptr, size);
}

static void fs_free(void *ptr) {
    runtime_free(ptr);
}

// IO HAL fallback
static int fs_putchar(int c) {
    (void)c;
    return c;
}

static int fs_getchar(void) {
    return -1; // EOF
}

static int fs_puts(const char *str) {
    (void)str;
    return 0;
}

static bool fs_kbhit(void) {
    return false;
}

static void fs_flush(void) {
}

static int fs_get_width(void) {
    return 80;
}

static int fs_get_height(void) {
    return 25;
}

static IoHandle fs_file_open(const char *path, const char *mode) {
    (void)path; (void)mode;
    return IO_HANDLE_INVALID;
}

static int fs_file_close(IoHandle handle) {
    (void)handle;
    return -1;
}

static size_t fs_file_read(IoHandle handle, void *buffer, size_t size, size_t count) {
    (void)handle; (void)buffer; (void)size; (void)count;
    return 0;
}

static size_t fs_file_write(IoHandle handle, const void *buffer, size_t size, size_t count) {
    (void)handle; (void)buffer; (void)size; (void)count;
    return 0;
}

static int fs_file_seek(IoHandle handle, int64_t offset, IoSeekOrigin origin) {
    (void)handle; (void)offset; (void)origin;
    return -1;
}

static int64_t fs_file_tell(IoHandle handle) {
    (void)handle;
    return -1;
}

static int fs_file_flush(IoHandle handle) {
    (void)handle;
    return -1;
}

static bool fs_file_eof(IoHandle handle) {
    (void)handle;
    return true;
}

static int64_t fs_file_size(const char *path) {
    (void)path;
    return -1;
}

static int fs_file_remove(const char *path) {
    (void)path;
    return -1;
}

static int fs_file_rename(const char *oldpath, const char *newpath) {
    (void)oldpath; (void)newpath;
    return -1;
}

static bool fs_file_exists(const char *path) {
    (void)path;
    return false;
}

// Time HAL fallback
static int64_t fs_now_epoch(void) {
    return 0;
}

static uint64_t fs_monotonic_ms(void) {
    static uint64_t sim_ms = 0;
    return sim_ms++;
}

static uint64_t fs_highres_ticks(void) {
    static uint64_t sim_ticks = 0;
    return sim_ticks++;
}

static uint64_t fs_ticks_freq(void) {
    return 1000;
}

static void fs_sleep_ms(uint32_t ms) {
    (void)ms;
}

// Audio HAL fallback
static bool fs_audio_init(void) { return true; }
static void fs_audio_shutdown(void) {}
static void fs_audio_beep(void) {}
static void fs_audio_tone(uint32_t freq, uint32_t dur) { (void)freq; (void)dur; }
static void fs_audio_stop(void) {}
static void fs_audio_vol(uint8_t v) { (void)v; }

// Video HAL fallback
static bool fs_video_init(int w, int h, int bpp, const char *t) { (void)w; (void)h; (void)bpp; (void)t; return false; }
static void fs_video_shutdown(void) {}
static void fs_video_present(const void *p, int w, int h, int pitch) { (void)p; (void)w; (void)h; (void)pitch; }
static void fs_video_palette(const uint32_t *pal, int c) { (void)pal; (void)c; }
static void fs_video_poll(void) {}
static bool fs_video_open(void) { return false; }

// Input HAL fallback
static bool fs_input_poll_key(int *out_key) { if (out_key) *out_key = 0; return false; }
static bool fs_input_poll_mouse(int *ox, int *oy, int *ob) { if (ox) *ox = 0; if (oy) *oy = 0; if (ob) *ob = 0; return false; }
static bool fs_input_is_key_down(int scancode) { (void)scancode; return false; }

void hal_init_freestanding(void) {
    global_hal.mem.alloc = fs_alloc;
    global_hal.mem.calloc = fs_calloc;
    global_hal.mem.realloc = fs_realloc;
    global_hal.mem.free = fs_free;
    global_hal.mem.lock = NULL;
    global_hal.mem.unlock = NULL;

    global_hal.io.console_putchar = fs_putchar;
    global_hal.io.console_getchar = fs_getchar;
    global_hal.io.console_puts = fs_puts;
    global_hal.io.console_kbhit = fs_kbhit;
    global_hal.io.console_flush = fs_flush;
    global_hal.io.console_get_width = fs_get_width;
    global_hal.io.console_get_height = fs_get_height;

    global_hal.io.file_open = fs_file_open;
    global_hal.io.file_close = fs_file_close;
    global_hal.io.file_read = fs_file_read;
    global_hal.io.file_write = fs_file_write;
    global_hal.io.file_seek = fs_file_seek;
    global_hal.io.file_tell = fs_file_tell;
    global_hal.io.file_flush = fs_file_flush;
    global_hal.io.file_eof = fs_file_eof;
    global_hal.io.file_size = fs_file_size;
    global_hal.io.file_remove = fs_file_remove;
    global_hal.io.file_rename = fs_file_rename;
    global_hal.io.file_exists = fs_file_exists;

    global_hal.time.now_epoch_seconds = fs_now_epoch;
    global_hal.time.monotonic_ms = fs_monotonic_ms;
    global_hal.time.highres_ticks = fs_highres_ticks;
    global_hal.time.ticks_frequency = fs_ticks_freq;
    global_hal.time.sleep_ms = fs_sleep_ms;

    global_hal.audio.init = fs_audio_init;
    global_hal.audio.shutdown = fs_audio_shutdown;
    global_hal.audio.beep = fs_audio_beep;
    global_hal.audio.tone = fs_audio_tone;
    global_hal.audio.stop = fs_audio_stop;
    global_hal.audio.set_volume = fs_audio_vol;

    global_hal.video.init = fs_video_init;
    global_hal.video.shutdown = fs_video_shutdown;
    global_hal.video.present_framebuffer = fs_video_present;
    global_hal.video.set_palette = fs_video_palette;
    global_hal.video.poll_events = fs_video_poll;
    global_hal.video.is_window_open = fs_video_open;

    global_hal.input.poll_key = fs_input_poll_key;
    global_hal.input.poll_mouse = fs_input_poll_mouse;
    global_hal.input.is_key_down = fs_input_is_key_down;
}
