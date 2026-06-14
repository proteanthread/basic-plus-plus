/*
 * ---
 * BASIC++ Interpreter - config_file.c
 * ---
 *
 * INI-style configuration file parser.
 *
 * FILE FORMAT:
 * Lines starting with ';' or '#' are comments.
 * Blank lines are ignored.
 * Key=value pairs are parsed case-insensitively.
 * Leading/trailing whitespace around keys and values is stripped.
 *
 * SUPPORTED KEYS:
 *   dialect   = PATB | GWBS | QBAS | EC55 | E116 | TRS1 | TRS2 | ...
 *   security  = OPEN | STANDARD | RESTRICTED
 *   strict    = ON | OFF
 *   quiet     = ON | OFF
 *
 * ---
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config_file.h"

/* Maximum line length in config file */
#define CFG_MAX_LINE 256

/*
 * config_file_get_name - Return the platform-specific filename.
 */
const char *config_file_get_name(void)
{
#if defined(__WATCOMC__) || defined(MSDOS) || defined(__DOS__)
    return "bpp.cfg";
#elif defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER)
    return "basicpp.cfg";
#else
    return "baspp.cfg";
#endif
}

/*
 * strip_whitespace - Strip leading/trailing whitespace in place.
 * Returns pointer to first non-whitespace character.
 * Trims trailing whitespace by inserting '\0'.
 */
static char *strip_whitespace(char *s)
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

/*
 * ci_equal - Case-insensitive string compare (C89-safe).
 */
static int ci_equal(const char *a, const char *b)
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

/*
 * parse_line - Parse a single key=value line into the config.
 */
static void parse_line(ConfigFile *cfg, const char *key,
                       const char *value)
{
    if (ci_equal(key, "dialect")) {
        int i;
        int len = (int)strlen(value);
        if (len > 15) len = 15;
        for (i = 0; i < len; i++) {
            char c = value[i];
            if (c >= 'a' && c <= 'z') c = (char)(c - 32);
            cfg->dialect[i] = c;
        }
        cfg->dialect[len] = '\0';
    } else if (ci_equal(key, "security")) {
        int i;
        int len = (int)strlen(value);
        if (len > 15) len = 15;
        for (i = 0; i < len; i++) {
            char c = value[i];
            if (c >= 'a' && c <= 'z') c = (char)(c - 32);
            cfg->security[i] = c;
        }
        cfg->security[len] = '\0';
    } else if (ci_equal(key, "strict")) {
        if (ci_equal(value, "on") || ci_equal(value, "yes") ||
            ci_equal(value, "1") || ci_equal(value, "true")) {
            cfg->strict = 1;
        } else {
            cfg->strict = 0;
        }
    } else if (ci_equal(key, "quiet")) {
        if (ci_equal(value, "on") || ci_equal(value, "yes") ||
            ci_equal(value, "1") || ci_equal(value, "true")) {
            cfg->quiet = 1;
        } else {
            cfg->quiet = 0;
        }
    }
    /* Unknown keys are silently ignored */
}

/*
 * try_open - Try to open a config file at a specific path.
 * Returns FILE* or NULL.
 */
static FILE *try_open(const char *dir, const char *filename,
                      char *out_path, int max_path)
{
    FILE *f;
    if (dir == NULL || dir[0] == '\0') {
        /* Try current directory */
        strncpy(out_path, filename, (size_t)(max_path - 1));
        out_path[max_path - 1] = '\0';
    } else {
        int dlen = (int)strlen(dir);
        int flen = (int)strlen(filename);
        if (dlen + flen + 2 > max_path) return NULL;
        strcpy(out_path, dir);
        /* Append separator if needed */
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

/*
 * config_file_load - Load configuration from the INI file.
 */
int config_file_load(ConfigFile *cfg)
{
    FILE *f = NULL;
    char line[CFG_MAX_LINE];
    const char *filename;
    const char *home;

    /* Initialize to defaults (all unset) */
    memset(cfg, 0, sizeof(ConfigFile));
    cfg->dialect[0] = '\0';
    cfg->security[0] = '\0';
    cfg->strict = -1;
    cfg->quiet = -1;
    cfg->found = 0;
    cfg->filepath[0] = '\0';

    filename = config_file_get_name();

    /* 1. Try current directory */
    f = try_open("", filename, cfg->filepath, 256);

    /* 2. Try home directory */
    if (f == NULL) {
#if defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER)
        home = getenv("USERPROFILE");
#else
        home = getenv("HOME");
#endif
        if (home != NULL) {
            f = try_open(home, filename,
                         cfg->filepath, 256);
        }
    }

    if (f == NULL) {
        cfg->filepath[0] = '\0';
        return -1; /* No config file found */
    }

    cfg->found = 1;

    /* Parse the file line by line */
    while (fgets(line, CFG_MAX_LINE, f) != NULL) {
        char *trimmed;
        char *eq;
        char *key;
        char *value;

        trimmed = strip_whitespace(line);

        /* Skip empty lines and comments */
        if (trimmed[0] == '\0' ||
            trimmed[0] == ';' ||
            trimmed[0] == '#') {
            continue;
        }

        /* Find the '=' separator */
        eq = strchr(trimmed, '=');
        if (eq == NULL) continue; /* malformed line */

        /* Split into key and value */
        *eq = '\0';
        key = strip_whitespace(trimmed);
        value = strip_whitespace(eq + 1);

        if (key[0] == '\0') continue;

        parse_line(cfg, key, value);
    }

    fclose(f);
    return 0;
}

/*
 * config_file_load_path - Load configuration from a specific file.
 */
int config_file_load_path(ConfigFile *cfg, const char *path)
{
    FILE *f;
    char line[CFG_MAX_LINE];

    /* Initialize to defaults (all unset) */
    memset(cfg, 0, sizeof(ConfigFile));
    cfg->dialect[0] = '\0';
    cfg->security[0] = '\0';
    cfg->strict = -1;
    cfg->quiet = -1;
    cfg->found = 0;
    cfg->filepath[0] = '\0';

    if (path == NULL || path[0] == '\0') {
        return -1;
    }

    f = fopen(path, "r");
    if (f == NULL) {
        return -1;
    }

    cfg->found = 1;
    strncpy(cfg->filepath, path, 255);
    cfg->filepath[255] = '\0';

    /* Parse the file line by line */
    while (fgets(line, CFG_MAX_LINE, f) != NULL) {
        char *trimmed;
        char *eq;
        char *key;
        char *value;

        trimmed = strip_whitespace(line);

        /* Skip empty lines and comments */
        if (trimmed[0] == '\0' ||
            trimmed[0] == ';' ||
            trimmed[0] == '#') {
            continue;
        }

        /* Find the '=' separator */
        eq = strchr(trimmed, '=');
        if (eq == NULL) continue;

        /* Split into key and value */
        *eq = '\0';
        key = strip_whitespace(trimmed);
        value = strip_whitespace(eq + 1);

        if (key[0] == '\0') continue;

        parse_line(cfg, key, value);
    }

    fclose(f);
    return 0;
}
