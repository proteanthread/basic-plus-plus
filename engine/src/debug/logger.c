// FILENAME: logger.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libboot, libcore, libengine, libkernel
// NEEDS: libcore (alloc.h, alloc.c, hal.h, logger.h, memops.h, memops.c)
// NEEDS: libcore (snprintf.h, snprintf.c, strops.h, strops.c)
// NEEDS: libplatform (platform.h)
// Provides core logic and interface definitions for logger within BASIC++.
//
// ---- Includes ----

#include "debug/logger.h"
#include "platform/platform.h"
#include <stdarg.h>
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "hal/hal.h"

// Global variables tracking logger state
static IoHandle g_log_file = IO_HANDLE_INVALID;
static IoHandle g_out_file = IO_HANDLE_INVALID;

static bool g_debug_mode = false;
static bool g_dry_run_mode = false;
static bool g_trace_active = false;

// Helper to get formatted current time string
static void get_timestamp_string(char *buf, size_t max_len) {
    HalContext *hal = hal_get();
    time_t raw_time = (time_t)(hal && hal->time.now_epoch_seconds ? hal->time.now_epoch_seconds() : 0);
    struct tm time_info;
    runtime_memset(&time_info, 0, sizeof(time_info));
    if (platform_localtime(&raw_time, &time_info)) {
        runtime_snprintf(buf, max_len, "%04d-%02d-%02d %02d:%02d:%02d",
                 time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday,
                 time_info.tm_hour, time_info.tm_min, time_info.tm_sec);
    } else {
        runtime_snprintf(buf, max_len, "0000-00-00 00:00:00");
    }
}

static void get_timestamped_log_filename(char *auto_log, size_t auto_log_size,
                                         char *auto_out, size_t auto_out_size) {
    char compact_time[64] = {0};
    HalContext *hal = hal_get();
    time_t raw_time = (time_t)(hal && hal->time.now_epoch_seconds ? hal->time.now_epoch_seconds() : 0);
    struct tm time_info;
    runtime_memset(&time_info, 0, sizeof(time_info));
    if (platform_localtime(&raw_time, &time_info)) {
        runtime_snprintf(compact_time, sizeof(compact_time), "%04d%02d%02d-%02d%02d%02d",
                 time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday,
                 time_info.tm_hour, time_info.tm_min, time_info.tm_sec);
    } else {
        runtime_snprintf(compact_time, sizeof(compact_time), "00000000-000000");
    }

    if (auto_log && auto_log_size > 0) {
        runtime_snprintf(auto_log, auto_log_size, "basic-session-%s.LOG", compact_time);
    }
    if (auto_out && auto_out_size > 0) {
        runtime_snprintf(auto_out, auto_out_size, "basic-session-%s.OUT", compact_time);
    }
}

static void log_write_file(IoHandle handle, const char *s, size_t len) {
    if (handle == IO_HANDLE_INVALID || !s || len == 0) return;
    HalContext *hal = hal_get();
    if (hal && hal->io.file_write) {
        hal->io.file_write(handle, s, 1, len);
    }
}

bool logger_init(const char *log_path, const char *out_path) {
    char time_str[64];
    get_timestamp_string(time_str, sizeof(time_str));
    HalContext *hal = hal_get();
    if (!hal || !hal->io.file_open) return false;

    // If log_path is provided, open it. Otherwise, generate a timestamped one
    if (log_path && log_path[0] != '\0') {
        g_log_file = hal->io.file_open(log_path, "w");
    } else if (g_debug_mode || g_dry_run_mode) {
        char auto_log[64];
        char auto_out[128];
        get_timestamped_log_filename(auto_log, sizeof(auto_log), auto_out, sizeof(auto_out));
        g_log_file = hal->io.file_open(auto_log, "w");
    }

    // If out_path is provided, open it
    if (out_path && out_path[0] != '\0') {
        g_out_file = hal->io.file_open(out_path, "w");
    } else if (g_debug_mode || g_dry_run_mode) {
        char auto_log[64];
        char auto_out[128];
        get_timestamped_log_filename(auto_log, sizeof(auto_log), auto_out, sizeof(auto_out));
        g_out_file = hal->io.file_open(auto_out, "w");
    }

    if (g_log_file != IO_HANDLE_INVALID) {
        char buf[256];
        int n = runtime_snprintf(buf, sizeof(buf), "[%s] [INFO] --- BASIC++ SESSION LOG INITIALIZED ---\n", time_str);
        if (n > 0) log_write_file(g_log_file, buf, (size_t)n);
    }
    return true;
}

void logger_close(void) {
    char time_str[64];
    get_timestamp_string(time_str, sizeof(time_str));
    HalContext *hal = hal_get();

    if (g_log_file != IO_HANDLE_INVALID) {
        char buf[256];
        int n = runtime_snprintf(buf, sizeof(buf), "[%s] [INFO] --- BASIC++ SESSION LOG TERMINATED ---\n", time_str);
        if (n > 0) log_write_file(g_log_file, buf, (size_t)n);
        if (hal && hal->io.file_close) hal->io.file_close(g_log_file);
        g_log_file = IO_HANDLE_INVALID;
    }
    if (g_out_file != IO_HANDLE_INVALID) {
        if (hal && hal->io.file_close) hal->io.file_close(g_out_file);
        g_out_file = IO_HANDLE_INVALID;
    }
}


static void log_format_and_write(const char *level, const char *fmt, va_list args) {
    if (g_log_file == IO_HANDLE_INVALID) return;
    char time_str[64];
    get_timestamp_string(time_str, sizeof(time_str));

    char header[128];
    int hlen = runtime_snprintf(header, sizeof(header), "[%s] [%s] ", time_str, level);
    if (hlen > 0) log_write_file(g_log_file, header, (size_t)hlen);

    char msg[1024];
    int mlen = runtime_vsnprintf(msg, sizeof(msg), fmt, args);
    if (mlen > 0) log_write_file(g_log_file, msg, (size_t)mlen);

    log_write_file(g_log_file, "\n", 1);
}

void log_info(const char *fmt, ...) {
    if (g_log_file == IO_HANDLE_INVALID) return;
    va_list args;
    va_start(args, fmt);
    log_format_and_write("INFO", fmt, args);
    va_end(args);
}

void log_warn(const char *fmt, ...) {
    if (g_log_file == IO_HANDLE_INVALID) return;
    va_list args;
    va_start(args, fmt);
    log_format_and_write("WARN", fmt, args);
    va_end(args);
}

void log_error(const char *fmt, ...) {
    if (g_log_file == IO_HANDLE_INVALID) return;
    va_list args;
    va_start(args, fmt);
    log_format_and_write("ERROR", fmt, args);
    va_end(args);
}

void log_write_out(const char *buf, size_t len) {
    if (g_out_file == IO_HANDLE_INVALID || !buf || len == 0) return;
    log_write_file(g_out_file, buf, len);
}


// Global Diagnostic State Accessors
void logger_set_debug(bool debug) {
    g_debug_mode = debug;
}

bool logger_is_debug(void) {
    return g_debug_mode;
}

void logger_set_dry_run(bool dry_run) {
    g_dry_run_mode = dry_run;
}

bool logger_is_dry_run(void) {
    return g_dry_run_mode;
}

void logger_set_trace(bool trace) {
    g_trace_active = trace;
}

bool logger_is_trace(void) {
    return g_trace_active;
}

