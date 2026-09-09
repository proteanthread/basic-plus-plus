// FILENAME: logger.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libboot, libcore, libengine, libkernel, libdevice
// NEEDS: platform, memory
// Provides multi-level logging, circular ring buffer, and pluggable sink interface within BASIC++.
//
// ---- Includes ----

#ifndef DEBUG_LOGGER_H
#define DEBUG_LOGGER_H

#include <stddef.h>
#include <stdbool.h>
#include "runtime/math/math.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BPP_LOG_LEVEL_DEFINED
#define BPP_LOG_LEVEL_DEFINED
// Log Severity Levels
typedef enum {
    BPP_LOG_TRACE = 0,
    BPP_LOG_DEBUG = 1,
    BPP_LOG_INFO  = 2,
    BPP_LOG_WARN  = 3,
    BPP_LOG_ERROR = 4,
    BPP_LOG_FATAL = 5
} BppLogLevel;

// Pluggable Log Sink Callback Function Signature
typedef void (*BppLogSinkFn)(BppLogLevel level, const char *tag, const char *message, const char *timestamp, void *userdata);
#endif

// @brief Initialize logging subsystems with paths.
// @param log_path Path to save system logs, or NULL for auto-generated name.
// @param out_path Path to save replicated console output, or NULL for auto-generated name.
// @return true on success, false if files could not be opened.
bool logger_init(const char *log_path, const char *out_path);

// @brief Close logging subsystems and release file handles.
void logger_close(void);

// @brief Attach a custom pluggable runtime_log sink.
// @param sink Callback function to invoke on runtime_log events.
// @param min_level Minimum severity level to route to this sink.
// @param userdata Optional user context pointer.
// @return true on success, false if sink table is full.
bool logger_add_sink(BppLogSinkFn sink, BppLogLevel min_level, void *userdata);

// @brief Detach a previously registered custom runtime_log sink.
void logger_remove_sink(BppLogSinkFn sink);

// @brief Set the global minimum logging severity threshold.
void logger_set_level(BppLogLevel level);

// @brief Get the global minimum logging severity threshold.
BppLogLevel logger_get_level(void);

// @brief Convert string name ("INFO", "DEBUG", etc.) to BppLogLevel.
BppLogLevel logger_level_from_str(const char *name);

// @brief Convert BppLogLevel to string name.
const char *logger_level_to_str(BppLogLevel level);

// @brief Emit structured runtime_log message with level and tag.
void log_emit(BppLogLevel level, const char *tag, const char *fmt, ...);

// Direct logging helpers
void log_trace(const char *fmt, ...);
void log_debug(const char *fmt, ...);
void log_info(const char *fmt, ...);
void log_warn(const char *fmt, ...);
void log_error(const char *fmt, ...);
void log_fatal(const char *fmt, ...);

// Circular Ring Buffer Diagnostics
void logger_ring_dump(BppLogLevel min_level, void (*print_fn)(const char *line));
void logger_ring_clear(void);
int  logger_ring_count(void);
const char *logger_ring_get_last(void);

// @brief Replicate raw console/input text to the .OUT file.
// @param buf Character buffer.
// @param len Buffer length in bytes.
void log_write_out(const char *buf, size_t len);

// Global Diagnostic State Accessors
void logger_set_debug(bool debug);
bool logger_is_debug(void);

void logger_set_dry_run(bool dry_run);
bool logger_is_dry_run(void);

void logger_set_trace(bool trace);
bool logger_is_trace(void);

#ifdef __cplusplus
}
#endif

#endif // DEBUG_LOGGER_H
