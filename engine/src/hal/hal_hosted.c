// FILENAME: hal_hosted.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (alloc.h, alloc.c, hal.h, hal_sdl2.h, hal_sdl2.c)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (time.h, time.c)
// NEEDS: libplatform (platform.h)
// Hosted Hardware Abstraction Layer dispatcher linking host libc and SDL2.
//
// ---- Includes ----

#include "hal/hal.h"
#include "hal/hal_sdl2.h"
#include "platform/platform.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/strops.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Global mutex for memory allocation
static BppMutex mem_mutex;
static bool mem_mutex_initialized = false;

static void hosted_mem_lock(void) {
    if (!mem_mutex_initialized) {
        platform_mutex_init(&mem_mutex);
        mem_mutex_initialized = true;
    }
    platform_mutex_lock(&mem_mutex);
}

static void hosted_mem_unlock(void) {
    if (mem_mutex_initialized) {
        platform_mutex_unlock(&mem_mutex);
    }
}

// Memory HAL implementation
static void *hosted_alloc(size_t size) {
    return malloc(size);
}

static void *hosted_calloc(size_t count, size_t size) {
    return calloc(count, size);
}

static void *hosted_realloc(void *ptr, size_t size) {
    return realloc(ptr, size);
}

static void hosted_free(void *ptr) {
    free(ptr);
}

// IO HAL implementation
static int hosted_putchar(int c) {
    return putchar(c);
}

static int hosted_getchar(void) {
    return platform_getch();
}

static int hosted_puts(const char *str) {
    return fputs(str, stdout);
}

static bool hosted_kbhit(void) {
    return platform_kbhit();
}

static void hosted_flush(void) {
    fflush(stdout);
}

static int hosted_get_width(void) {
    return platform_console_width();
}

static int hosted_get_height(void) {
    return platform_console_height();
}

static FILE *hosted_get_file_ptr(IoHandle handle) {
    if (handle == IO_HANDLE_INVALID) return NULL;
    if (handle == IO_STDIN_HANDLE) return stdin;
    if (handle == IO_STDOUT_HANDLE) return stdout;
    if (handle == IO_STDERR_HANDLE) return stderr;
    return (FILE *)handle;
}

static IoHandle hosted_file_open(const char *path, const char *mode) {
    if (!path || !mode) return IO_HANDLE_INVALID;
    FILE *fp = fopen(path, mode);
    return fp ? (IoHandle)fp : IO_HANDLE_INVALID;
}

static int hosted_file_close(IoHandle handle) {
    if (handle == IO_HANDLE_INVALID || handle == IO_STDIN_HANDLE || handle == IO_STDOUT_HANDLE || handle == IO_STDERR_HANDLE) return 0;
    return fclose((FILE *)handle);
}

static size_t hosted_file_read(IoHandle handle, void *buffer, size_t size, size_t count) {
    FILE *fp = hosted_get_file_ptr(handle);
    if (!fp || !buffer) return 0;
    return fread(buffer, size, count, fp);
}

static size_t hosted_file_write(IoHandle handle, const void *buffer, size_t size, size_t count) {
    FILE *fp = hosted_get_file_ptr(handle);
    if (!fp || !buffer) return 0;
    return fwrite(buffer, size, count, fp);
}

static int hosted_file_seek(IoHandle handle, int64_t offset, IoSeekOrigin origin) {
    FILE *fp = hosted_get_file_ptr(handle);
    if (!fp) return -1;
    int c_origin = SEEK_SET;
    if (origin == IO_SEEK_CUR) c_origin = SEEK_CUR;
    else if (origin == IO_SEEK_END) c_origin = SEEK_END;
#if defined(_WIN32)
    return _fseeki64(fp, offset, c_origin);
#else
    return fseeko(fp, (off_t)offset, c_origin);
#endif
}

static int64_t hosted_file_tell(IoHandle handle) {
    FILE *fp = hosted_get_file_ptr(handle);
    if (!fp) return -1;
#if defined(_WIN32)
    return _ftelli64(fp);
#else
    return (int64_t)ftello(fp);
#endif
}

static int hosted_file_flush(IoHandle handle) {
    FILE *fp = hosted_get_file_ptr(handle);
    if (!fp) return -1;
    return fflush(fp);
}

static bool hosted_file_eof(IoHandle handle) {
    FILE *fp = hosted_get_file_ptr(handle);
    if (!fp) return true;
    return feof(fp) != 0;
}

static int64_t hosted_file_size(const char *path) {
    return (int64_t)platform_filesize(path);

}

static int hosted_file_remove(const char *path) {
    return platform_remove(path);
}

static int hosted_file_rename(const char *oldpath, const char *newpath) {
    return platform_rename(oldpath, newpath);
}

static bool hosted_file_exists(const char *path) {
    return platform_filesize(path) >= 0;
}

// Time HAL implementation
static int64_t hosted_now_epoch(void) {
    return (int64_t)time(NULL);
}

static uint64_t hosted_monotonic_ms(void) {
    return (uint64_t)(platform_get_highres_time() * 1000.0);
}

static uint64_t hosted_highres_ticks(void) {
    return (uint64_t)(platform_get_highres_time() * 1000000.0);
}

static uint64_t hosted_ticks_freq(void) {
    return 1000000;
}

static void hosted_sleep_ms(uint32_t ms) {
    platform_sleep_ms(ms);
}

// Audio HAL implementation
static bool hosted_audio_init(void) { return true; }
static void hosted_audio_shutdown(void) { platform_sound_stop(); }
static void hosted_audio_beep(void) { platform_sound_beep(); }
static void hosted_audio_tone(uint32_t freq, uint32_t dur) { platform_sound_tone(freq, dur); }
static void hosted_audio_stop(void) { platform_sound_stop(); }
static void hosted_audio_vol(uint8_t v) { (void)v; }

// Video HAL placeholder (wired to SDL/BGI in libadvanced)
static bool hosted_video_init(int w, int h, int bpp, const char *t) { (void)w; (void)h; (void)bpp; (void)t; return false; }
static void hosted_video_shutdown(void) {}
static void hosted_video_present(const void *p, int w, int h, int pitch) { (void)p; (void)w; (void)h; (void)pitch; }
static void hosted_video_palette(const uint32_t *pal, int c) { (void)pal; (void)c; }
static void hosted_video_poll(void) {}
static bool hosted_video_open(void) { return false; }

// Input HAL implementation
static bool hosted_input_poll_key(int *out_key) {
    if (platform_kbhit()) {
        if (out_key) *out_key = platform_getch();
        return true;
    }
    return false;
}

static bool hosted_input_poll_mouse(int *ox, int *oy, int *ob) { (void)ox; (void)oy; (void)ob; return false; }
static bool hosted_input_is_key_down(int scancode) { (void)scancode; return false; }

void hal_init_hosted(void) {
    platform_init();
    HalContext *hal = hal_get();

    hal->mem.alloc = hosted_alloc;
    hal->mem.calloc = hosted_calloc;
    hal->mem.realloc = hosted_realloc;
    hal->mem.free = hosted_free;
    hal->mem.lock = hosted_mem_lock;
    hal->mem.unlock = hosted_mem_unlock;

    hal->io.console_putchar = hosted_putchar;
    hal->io.console_getchar = hosted_getchar;
    hal->io.console_puts = hosted_puts;
    hal->io.console_kbhit = hosted_kbhit;
    hal->io.console_flush = hosted_flush;
    hal->io.console_get_width = hosted_get_width;
    hal->io.console_get_height = hosted_get_height;

    hal->io.file_open = hosted_file_open;
    hal->io.file_close = hosted_file_close;
    hal->io.file_read = hosted_file_read;
    hal->io.file_write = hosted_file_write;
    hal->io.file_seek = hosted_file_seek;
    hal->io.file_tell = hosted_file_tell;
    hal->io.file_flush = hosted_file_flush;
    hal->io.file_eof = hosted_file_eof;
    hal->io.file_size = hosted_file_size;
    hal->io.file_remove = hosted_file_remove;
    hal->io.file_rename = hosted_file_rename;
    hal->io.file_exists = hosted_file_exists;

    hal->time.now_epoch_seconds = hosted_now_epoch;
    hal->time.monotonic_ms = hosted_monotonic_ms;
    hal->time.highres_ticks = hosted_highres_ticks;
    hal->time.ticks_frequency = hosted_ticks_freq;
    hal->time.sleep_ms = hosted_sleep_ms;

    hal->audio.init = hosted_audio_init;
    hal->audio.shutdown = hosted_audio_shutdown;
    hal->audio.beep = hosted_audio_beep;
    hal->audio.tone = hosted_audio_tone;
    hal->audio.stop = hosted_audio_stop;
    hal->audio.set_volume = hosted_audio_vol;

    hal->video.init = hosted_video_init;
    hal->video.shutdown = hosted_video_shutdown;
    hal->video.present_framebuffer = hosted_video_present;
    hal->video.set_palette = hosted_video_palette;
    hal->video.poll_events = hosted_video_poll;
    hal->video.is_window_open = hosted_video_open;

    hal->input.poll_key = hosted_input_poll_key;
    hal->input.poll_mouse = hosted_input_poll_mouse;
    hal->input.is_key_down = hosted_input_is_key_down;

    // Attach SDL2 drivers for video and audio if enabled
    hal_init_sdl2();

    // Also configure memory subsystem hooks
    RuntimeMemHooks hooks;
    hooks.malloc = hosted_alloc;
    hooks.calloc = hosted_calloc;
    hooks.realloc = hosted_realloc;
    hooks.free = hosted_free;
    hooks.lock = hosted_mem_lock;
    hooks.unlock = hosted_mem_unlock;
    runtime_mem_set_hooks(&hooks);
}
