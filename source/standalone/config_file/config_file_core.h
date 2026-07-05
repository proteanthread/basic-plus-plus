/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: config_file_core.h
 * Subsystem: INI Configuration File Scanner
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Scans INI files (basicpp.cfg) for dialect, security, memory, and module properties.
 *
 * 2. WHAT TO EXPECT:
 *    Parses keys/values and resolves path overrides.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Parser limits, home/local directory path searches.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    INI syntax grammar constraints.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If properties are ignored, verify formatting and file permissions.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE INI CONFIGURATION FILE PARSER ENGINE
 * File: config_file_core.h
 * ===================================================================== */

#ifndef STANDALONE_CONFIG_FILE_CORE_H
#define STANDALONE_CONFIG_FILE_CORE_H

#include <stdio.h>

/* Callback type triggered for every parsed key-value pair. */
typedef void (*ConfigFileCoreKvCb)(void *user_data, const char *key, const char *value);

/* Find and parse config file using default search rules (CWD, home folder)
 * triggered by executable filename base.
 *
 * Parameters:
 *   exe_path - path of the binary (used to formulate config filename like basicpp.cfg)
 *   kv_cb - callback triggered on each parsed key-value pair
 *   user_data - custom context passed to the callback
 *   out_filepath - buffer to write the loaded config file absolute path (can be NULL)
 *   max_filepath - size of out_filepath buffer
 *
 * Returns 0 on success (or if file not found, but it initialized), -1 on error.
 */
int config_file_core_load(const char *exe_path, ConfigFileCoreKvCb kv_cb, void *user_data, char *out_filepath, int max_filepath);

/* Load and parse config from a specific path.
 *
 * Returns 0 on success, -1 if the file cannot be opened.
 */
int config_file_core_load_path(const char *path, ConfigFileCoreKvCb kv_cb, void *user_data);

/* Core utility functions exposed to simplify parsing/string operations */
char *config_file_core_strip_whitespace(char *s);
int config_file_core_ci_equal(const char *a, const char *b);
void config_file_core_get_name(const char *exe_path, char *out_name, int max_len);

#endif /* STANDALONE_CONFIG_FILE_CORE_H */
