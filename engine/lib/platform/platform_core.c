// FILENAME: platform_core.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (string.h)
// NEEDS: libengine (string.c, time.h, time.c)
// NEEDS: libplatform (platform.h)
// Provides cross-platform OS abstraction primitives for platform_core.
//
// ---- Includes ----

#include "platform/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <sys/timeb.h>
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

static double g_boot_time = 0.0;

static double platform_get_epoch_time(void) {
#if defined(_WIN32)
    struct _timeb tb;
    _ftime(&tb);
    return (double)tb.time + ((double)tb.millitm / 1000.0);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + ((double)tv.tv_usec / 1000000.0);
#endif
}

void platform_init(void) {
    g_boot_time = platform_get_epoch_time();
}

void platform_shutdown(void) {
    // Cleanup core platform hooks if any
}

BppPlatformId platform_get_id(void) {
#if defined(_WIN32)
    return PLAT_WINDOWS;
#elif defined(__WATCOMC__) || defined(MSDOS)
    return PLAT_DOS;
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    return PLAT_POSIX;
#else
    return PLAT_UNKNOWN;
#endif
}

const char *platform_name(void) {
#if defined(_WIN32)
    return "Windows";
#elif defined(__WATCOMC__) || defined(MSDOS)
    return "FreeDOS";
#elif defined(__linux__)
    return "Linux";
#elif defined(__APPLE__)
    return "macOS";
#elif defined(__FreeBSD__)
    return "FreeBSD";
#elif defined(__OpenBSD__)
    return "OpenBSD";
#elif defined(__NetBSD__)
    return "NetBSD";
#else
    return "Unknown OS";
#endif
}

void platform_sleep_ms(uint32_t ms) {
#if defined(_WIN32)
    Sleep(ms);
#elif defined(__WATCOMC__) || defined(MSDOS)
    delay(ms);
#else
    usleep(ms * 1000);
#endif
}

double platform_get_timer(void) {
    return platform_get_epoch_time() - g_boot_time;
}

void platform_fatal(const char *msg) {
    if (msg) {
        fprintf(stderr, "FATAL: %s\n", msg);
    }
}

void platform_panic(int code, const char *msg) {
    if (msg) {
        fprintf(stderr, "PANIC [%d]: %s\n", code, msg);
    }
}
