// FILENAME: logger.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libboot, libcore, libengine, libkernel, libdevice
// NEEDS: libcore (alloc.h, alloc.c, hal.h, logger.h, memops.h, memops.c)
// NEEDS: libcore (runtime_snprintf.h, runtime_snprintf.c, strops.h, strops.c)
// NEEDS: libplatform (platform.h)
// Provides multi-level logging, circular ring buffer, and pluggable sink interface within BASIC++.
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

#define MAX_LOG_SINKS 8
#define LOG_RING_CAPACITY 1024
#define LOG_MSG_MAX_LEN 512

typedef struct {
    BppLogLevel level;
    char timestamp[32];
    char tag[32];
    char message[LOG_MSG_MAX_LEN];
} LogEntry;

typedef struct {
    BppLogSinkFn fn;
    BppLogLevel min_level;
    void *userdata;
    bool active;
} RegisteredSink;

// Global variables tracking logger state
static IoHandle g_log_file = IO_HANDLE_INVALID;
static IoHandle g_out_file = IO_HANDLE_INVALID;

static bool g_debug_mode = false;
static bool g_dry_run_mode = false;
static bool g_trace_active = false;
static BppLogLevel g_min_log_level = BPP_LOG_INFO;

static RegisteredSink g_sinks[MAX_LOG_SINKS];
static int g_sink_count = 0;

// In-Memory Circular Ring Buffer
static LogEntry g_ring_buffer[LOG_RING_CAPACITY];
static int g_ring_head = 0;
static int g_ring_count = 0;

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

bool logger_add_sink(BppLogSinkFn sink, BppLogLevel min_level, void *userdata) {
    if (!sink) return false;
    for (int i = 0; i < MAX_LOG_SINKS; i++) {
        if (!g_sinks[i].active || g_sinks[i].fn == sink) {
            g_sinks[i].fn = sink;
            g_sinks[i].min_level = min_level;
            g_sinks[i].userdata = userdata;
            g_sinks[i].active = true;
            return true;
        }
    }
    return false;
}

void logger_remove_sink(BppLogSinkFn sink) {
    if (!sink) return;
    for (int i = 0; i < MAX_LOG_SINKS; i++) {
        if (g_sinks[i].active && g_sinks[i].fn == sink) {
            g_sinks[i].active = false;
            g_sinks[i].fn = NULL;
        }
    }
}

void logger_set_level(BppLogLevel level) {
    g_min_log_level = level;
    if (level == BPP_LOG_TRACE) g_trace_active = true;
    if (level == BPP_LOG_DEBUG) g_debug_mode = true;
}

BppLogLevel logger_get_level(void) {
    return g_min_log_level;
}

BppLogLevel logger_level_from_str(const char *name) {
    if (!name) return BPP_LOG_INFO;
    if (runtime_strcasecmp(name, "TRACE") == 0) return BPP_LOG_TRACE;
    if (runtime_strcasecmp(name, "DEBUG") == 0) return BPP_LOG_DEBUG;
    if (runtime_strcasecmp(name, "INFO") == 0)  return BPP_LOG_INFO;
    if (runtime_strcasecmp(name, "WARN") == 0 || runtime_strcasecmp(name, "WARNING") == 0) return BPP_LOG_WARN;
    if (runtime_strcasecmp(name, "ERROR") == 0 || runtime_strcasecmp(name, "ERR") == 0) return BPP_LOG_ERROR;
    if (runtime_strcasecmp(name, "FATAL") == 0) return BPP_LOG_FATAL;
    return BPP_LOG_INFO;
}

const char *logger_level_to_str(BppLogLevel level) {
    switch (level) {
        case BPP_LOG_TRACE: return "TRACE";
        case BPP_LOG_DEBUG: return "DEBUG";
        case BPP_LOG_INFO:  return "INFO";
        case BPP_LOG_WARN:  return "WARN";
        case BPP_LOG_ERROR: return "ERROR";
        case BPP_LOG_FATAL: return "FATAL";
        default:            return "INFO";
    }
}

void log_emit(BppLogLevel level, const char *tag, const char *fmt, ...) {
    if (level < g_min_log_level) return;

    char time_str[32];
    get_timestamp_string(time_str, sizeof(time_str));

    char msg[LOG_MSG_MAX_LEN];
    va_list args;
    va_start(args, fmt);
    runtime_vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    // 1. Store into In-Memory Circular Ring Buffer
    int idx = (g_ring_head + g_ring_count) % LOG_RING_CAPACITY;
    if (g_ring_count == LOG_RING_CAPACITY) {
        g_ring_head = (g_ring_head + 1) % LOG_RING_CAPACITY;
    } else {
        g_ring_count++;
    }

    g_ring_buffer[idx].level = level;
    runtime_snprintf(g_ring_buffer[idx].timestamp, sizeof(g_ring_buffer[idx].timestamp), "%s", time_str);
    runtime_snprintf(g_ring_buffer[idx].tag, sizeof(g_ring_buffer[idx].tag), "%s", tag ? tag : "SYS");
    runtime_snprintf(g_ring_buffer[idx].message, sizeof(g_ring_buffer[idx].message), "%s", msg);

    // 2. Write to active .LOG file if opened
    if (g_log_file != IO_HANDLE_INVALID && level >= g_min_log_level) {
        char line[1024];
        int n = runtime_snprintf(line, sizeof(line), "[%s] [%s] [%s] %s\n",
                                 time_str, logger_level_to_str(level), tag ? tag : "SYS", msg);
        if (n > 0) log_write_file(g_log_file, line, (size_t)n);
    }

    // 3. Dispatch to registered custom sinks
    for (int i = 0; i < MAX_LOG_SINKS; i++) {
        if (g_sinks[i].active && g_sinks[i].fn && level >= g_sinks[i].min_level) {
            g_sinks[i].fn(level, tag ? tag : "SYS", msg, time_str, g_sinks[i].userdata);
        }
    }
}

void log_trace(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char msg[LOG_MSG_MAX_LEN];
    runtime_vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);
    log_emit(BPP_LOG_TRACE, "SYS", "%s", msg);
}

void log_debug(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char msg[LOG_MSG_MAX_LEN];
    runtime_vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);
    log_emit(BPP_LOG_DEBUG, "SYS", "%s", msg);
}

void log_info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char msg[LOG_MSG_MAX_LEN];
    runtime_vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);
    log_emit(BPP_LOG_INFO, "SYS", "%s", msg);
}

void log_warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char msg[LOG_MSG_MAX_LEN];
    runtime_vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);
    log_emit(BPP_LOG_WARN, "SYS", "%s", msg);
}

void log_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char msg[LOG_MSG_MAX_LEN];
    runtime_vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);
    log_emit(BPP_LOG_ERROR, "SYS", "%s", msg);
}

void log_fatal(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char msg[LOG_MSG_MAX_LEN];
    runtime_vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);
    log_emit(BPP_LOG_FATAL, "SYS", "%s", msg);
}

void logger_ring_dump(BppLogLevel min_level, void (*print_fn)(const char *line)) {
    if (!print_fn) return;
    for (int i = 0; i < g_ring_count; i++) {
        int idx = (g_ring_head + i) % LOG_RING_CAPACITY;
        if (g_ring_buffer[idx].level >= min_level) {
            char line[1024];
            runtime_snprintf(line, sizeof(line), "[%s] [%s] [%s] %s",
                             g_ring_buffer[idx].timestamp,
                             logger_level_to_str(g_ring_buffer[idx].level),
                             g_ring_buffer[idx].tag,
                             g_ring_buffer[idx].message);
            print_fn(line);
        }
    }
}

void logger_ring_clear(void) {
    g_ring_head = 0;
    g_ring_count = 0;
    runtime_memset(g_ring_buffer, 0, sizeof(g_ring_buffer));
}

int logger_ring_count(void) {
    return g_ring_count;
}

const char *logger_ring_get_last(void) {
    if (g_ring_count == 0) return "";
    int last_idx = (g_ring_head + g_ring_count - 1) % LOG_RING_CAPACITY;
    return g_ring_buffer[last_idx].message;
}

void log_write_out(const char *buf, size_t len) {
    if (g_out_file == IO_HANDLE_INVALID || !buf || len == 0) return;
    log_write_file(g_out_file, buf, len);
}

// Global Diagnostic State Accessors
void logger_set_debug(bool debug) {
    g_debug_mode = debug;
    if (debug && g_min_log_level > BPP_LOG_DEBUG) g_min_log_level = BPP_LOG_DEBUG;
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
    if (trace && g_min_log_level > BPP_LOG_TRACE) g_min_log_level = BPP_LOG_TRACE;
}

bool logger_is_trace(void) {
    return g_trace_active;
}
