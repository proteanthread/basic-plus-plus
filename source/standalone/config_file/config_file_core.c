/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: config_file_core.c
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
 * File: config_file_core.c
 * ===================================================================== */

#include "config_file_core.h"
#include <stdlib.h>
#include <string.h>

#define CFG_MAX_LINE 256

char *config_file_core_strip_whitespace(char *s)
{
    char *end;
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '\0') return s;
    end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' ||
           *end == '\n' || *end == '\r'))
        *end-- = '\0';
    return s;
}

int config_file_core_ci_equal(const char *a, const char *b)
{
    while (*a && *b) {
        char ca = *a, cb = *b;
        if (ca >= 'a' && ca <= 'z') ca = (char)(ca - 32);
        if (cb >= 'a' && cb <= 'z') cb = (char)(cb - 32);
        if (ca != cb) return 0;
        a++; b++;
    }
    return (*a == '\0' && *b == '\0');
}

void config_file_core_get_name(const char *exe_path, char *out_name, int max_len)
{
    const char *base = exe_path;
    const char *p;
    int len;

    if (!exe_path || !*exe_path) {
        strncpy(out_name, "basicpp.cfg", (size_t)max_len - 1);
        out_name[max_len - 1] = '\0';
        return;
    }

    // Find basename
    for (p = exe_path; *p; p++) {
        if (*p == '/' || *p == '\\') {
            base = p + 1;
        }
    }

    // Copy up to the dot or end
    len = 0;
    while (base[len] && base[len] != '.' && len < (max_len - 5)) {
        out_name[len] = base[len];
        len++;
    }
    out_name[len] = '\0';
    strcat(out_name, ".cfg");
}

static FILE *try_open(const char *dir, const char *filename,
                      char *out_path, int max_path)
{
    FILE *f;
    if (dir == NULL || dir[0] == '\0') {
        // Try current directory
        strncpy(out_path, filename, (size_t)(max_path - 1));
        out_path[max_path - 1] = '\0';
    } else {
        int dlen = (int)strlen(dir);
        int flen = (int)strlen(filename);
        if (dlen + flen + 2 > max_path) return NULL;
        strcpy(out_path, dir);
        // Append separator if needed
#if defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER) || \
    defined(__WATCOMC__) || defined(MSDOS)
        if (out_path[dlen - 1] != '\\' &&
            out_path[dlen - 1] != '/') {
            out_path[dlen] = '\\';
            out_path[dlen + 1] = '\0';
        }
#else
        if (out_path[dlen - 1] != '/') {
            out_path[dlen] = '/';
            out_path[dlen + 1] = '\0';
        }
#endif
        strcat(out_path, filename);
    }
    f = fopen(out_path, "r");
    return f;
}

static int parse_file_stream(FILE *f, ConfigFileCoreKvCb kv_cb, void *user_data)
{
    char line[CFG_MAX_LINE];
    while (fgets(line, CFG_MAX_LINE, f) != NULL) {
        char *trimmed;
        char *eq;
        char *key;
        char *value;

        trimmed = config_file_core_strip_whitespace(line);

        // Skip empty lines and comments
        if (trimmed[0] == '\0' ||
            trimmed[0] == ';' ||
            trimmed[0] == '#') {
            continue;
        }

        // Find the '=' separator
        eq = strchr(trimmed, '=');
        if (eq == NULL) continue; // malformed line

        // Split into key and value
        *eq = '\0';
        key = config_file_core_strip_whitespace(trimmed);
        value = config_file_core_strip_whitespace(eq + 1);

        if (key[0] == '\0') continue;

        if (kv_cb) {
            kv_cb(user_data, key, value);
        }
    }
    return 0;
}

int config_file_core_load(const char *exe_path, ConfigFileCoreKvCb kv_cb, void *user_data, char *out_filepath, int max_filepath)
{
    FILE *f = NULL;
    char filename[256];
    const char *home;
    char local_filepath[256];
    char *filepath_ptr = out_filepath ? out_filepath : local_filepath;
    int filepath_max = out_filepath ? max_filepath : 256;

    config_file_core_get_name(exe_path, filename, sizeof(filename));

    // 1. Try current directory
    f = try_open("", filename, filepath_ptr, filepath_max);

    // 2. Try home directory
    if (f == NULL) {
#if defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER)
        home = getenv("USERPROFILE");
#else
        home = getenv("HOME");
#endif
        if (home != NULL) {
            f = try_open(home, filename, filepath_ptr, filepath_max);
        }
    }

    if (f == NULL) {
        if (out_filepath && max_filepath > 0) {
            out_filepath[0] = '\0';
        }
        return -1;
    }

    parse_file_stream(f, kv_cb, user_data);
    fclose(f);
    return 0;
}

int config_file_core_load_path(const char *path, ConfigFileCoreKvCb kv_cb, void *user_data)
{
    FILE *f;
    if (path == NULL || path[0] == '\0') {
        return -1;
    }
    f = fopen(path, "r");
    if (f == NULL) {
        return -1;
    }
    parse_file_stream(f, kv_cb, user_data);
    fclose(f);
    return 0;
}
