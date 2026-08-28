// FILENAME: logger.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libboot, libcore, libengine, libkernel
// NEEDS: platform, memory
// Provides core logic and interface definitions for logger within BASIC++.
//
// ---- Includes ----

#ifndef DEBUG_LOGGER_H
#define DEBUG_LOGGER_H

#include <stddef.h>
#include <stdbool.h>

// @brief Initialize logging subsystems with paths.
// @param log_path Path to save system logs, or NULL for auto-generated name.
// @param out_path Path to save replicated console output, or NULL for auto-generated name.
// @return true on success, false if files could not be opened.
bool logger_init(const char *log_path, const char *out_path);

// @brief Close logging subsystems and release file handles.
void logger_close(void);

// @brief Log informative runtime events.
void log_info(const char *fmt, ...);

// @brief Log warnings or non-fatal engine anomalies.
void log_warn(const char *fmt, ...);

// @brief Log fatal or trapped runtime errors.
void log_error(const char *fmt, ...);

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

#endif // DEBUG_LOGGER_H
