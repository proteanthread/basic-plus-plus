/**
 * @file bpp_logger.h
 * @brief Diagnostic Session Logging & Redirection API.
 *
 * SECTION 1: WHAT IT DOES
 * - Declares the public interface for the session logging and console redirection framework.
 * - Manages logger lifetime, level-specific output (.LOG files), and console mirroring (.OUT files).
 *
 * SECTION 2: WHY IT EXISTS
 * - Allows tracking of boot stages, parser diagnostic metadata, and runtime warning/error traces.
 * - Replicates console output and user input to support automated debugging and testing feedback.
 *
 * SECTION 3: WHY IT WORKS THIS WAY
 * - Decouples log recording from stdio, routing messages to dedicated file descriptors or stdout when in debug mode.
 * - Integrates with VDev writes (CON:) to mirror console output without polluting virtual machine registers.
 *
 * SECTION 4: WHAT CAN BE CHANGED
 * - Formatting of log headers, timestamps, or default file extensions.
 *
 * SECTION 5: WHAT CANNOT BE CHANGED
 * - Thread-safe logging structures or global diagnostic flags.
 *
 * SECTION 6: WHAT TO EXPECT
 * - Low-overhead file writes. Output is flushed immediately on warnings and errors to prevent loss during crashes.
 *
 * SECTION 7: WHAT TO DO IF SOMETHING BREAKS
 * - Verify file permissions in the target workspace and ensure folder exists.
 *
 * SECTION 8: ASSUMPTIONS
 * - The workspace configuration/sandbox permits write operations to log files.
 *
 * SECTION 9: PORTABILITY CONCERNS
 * - Relies on standard C file stream interfaces; path separators are normalized using platform helpers.
 *
 * SECTION 10: FUTURE EXPANSIONS
 * - Real-time network stream logging (e.g. syslog or remote UDP logs).
 *
 * SECTION 11: EXTERNAL EXTENSION HOOKS
 * - Plugins can log errors and operational warnings using the standard bpp_log_* interfaces.
 */

#ifndef BPP_LOGGER_H
#define BPP_LOGGER_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Initialize logging subsystems with paths.
 * @param log_path Path to save system logs, or NULL for auto-generated name.
 * @param out_path Path to save replicated console output, or NULL for auto-generated name.
 * @return true on success, false if files could not be opened.
 */
bool bpp_logger_init(const char *log_path, const char *out_path);

/**
 * @brief Close logging subsystems and release file handles.
 */
void bpp_logger_close(void);

/**
 * @brief Log informative runtime events.
 */
void bpp_log_info(const char *fmt, ...);

/**
 * @brief Log warnings or non-fatal engine anomalies.
 */
void bpp_log_warn(const char *fmt, ...);

/**
 * @brief Log fatal or trapped runtime errors.
 */
void bpp_log_error(const char *fmt, ...);

/**
 * @brief Replicate raw console/input text to the .OUT file.
 * @param buf Character buffer.
 * @param len Buffer length in bytes.
 */
void bpp_log_write_out(const char *buf, size_t len);

/* Global Diagnostic State Accessors */
void bpp_logger_set_debug(bool debug);
bool bpp_logger_is_debug(void);

void bpp_logger_set_dry_run(bool dry_run);
bool bpp_logger_is_dry_run(void);

void bpp_logger_set_trace(bool trace);
bool bpp_logger_is_trace(void);

#endif /* BPP_LOGGER_H */
