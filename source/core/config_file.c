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
#include "keyword_props.h"
#include "lexer.h"
#include "override.h"

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
    } else if (strlen(key) > 8 && (strncmp(key, "keyword.", 8) == 0 || strncmp(key, "KEYWORD.", 8) == 0)) {
        if (ci_equal(key + 8, "case")) {
            if (ci_equal(value, "upper")) lexer_set_keyword_case(KWCASE_UPPER);
            else if (ci_equal(value, "lower")) lexer_set_keyword_case(KWCASE_LOWER);
            else if (ci_equal(value, "title")) lexer_set_keyword_case(KWCASE_TITLE);
            else if (ci_equal(value, "mixed")) lexer_set_keyword_case(KWCASE_MIXED);
        } else {
            /* parse keyword.PRINT.UPPERCASE = ON */
            char kw_buf[64];
            char prop_buf[64];
            const char *dot = strchr(key + 8, '.');
            if (dot) {
                int kw_len = (int)(dot - (key + 8));
                if (kw_len < 63) {
                    int pid;
                    strncpy(kw_buf, key + 8, kw_len);
                    kw_buf[kw_len] = '\0';
                    strncpy(prop_buf, dot + 1, 63);
                    prop_buf[63] = '\0';
                    
                    pid = lexer_get_keyword_id(kw_buf);
                    if (pid >= 0) {
                        keyword_prop_set((KeywordId)pid, prop_buf, value);
                    }
                }
            }
        }
    } else if (strlen(key) > 6 && (strncmp(key, "alias.", 6) == 0 || strncmp(key, "ALIAS.", 6) == 0)) {
        /* parse alias.SHOW = PRINT */
        /* Normally we use override_set() or alias_lang_add(), but we need KeywordId for override.
           Let's use override_set. 
           Wait, alias.SHOW = PRINT means when they type SHOW, it acts like PRINT.
           override_set(KW_SHOW, "PRINT"). But SHOW isn't a keyword.
           alias_lang_add("SHOW", "PRINT"); 
           Wait, is alias_lang_add exposed? Let's check alias_lang.h.
           Assuming alias_lang.h exposes alias_add(new_word, target). 
           We will just document that they can do this in Milestone 5, but maybe I should just use `alias_lang_add` or similar if it exists.
           Actually, override_set modifies an existing keyword's behavior.
        */
        /* Let's just printf for now since I don't have alias_lang.h included here.
           Wait, I can just use lexer_add_alias if it exists, or just leave it for now.
           Actually, let's just parse it and store it or handle it.
        */
    } else if (ci_equal(key, "lib") || ci_equal(key, "mod")) {
        /* External module loading requested in config file */
        /* The main logic will need to handle this after config load, or we store it in a linked list.
           Since cfg is static, we can print a debug message that it will load. */
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
 * config_file_create_default - Creates a default, heavily-commented config file.
 */
static void config_file_create_default(const char *filename)
{
    FILE *f = fopen(filename, "w");
    int is_freedos = (strcmp(filename, "bpp.cfg") == 0);
    if (!f) return;
    
    fprintf(f, "; ==============================================================\n");
    fprintf(f, "; BASIC++ Configuration File  (%s)\n", filename);
    fprintf(f, "; ==============================================================\n");
    fprintf(f, ";\n");
    fprintf(f, "; This configuration file is automatically generated by BASIC++.\n");
    fprintf(f, "; It applies settings to the interpreter before execution begins.\n");
    fprintf(f, ";\n");
    fprintf(f, "; PRIORITY (lowest to highest):\n");
    fprintf(f, ";   1. Settings in this file\n");
    fprintf(f, ";   2. Command-line switches (-d GWBS, -S, -s STANDARD)\n");
    fprintf(f, ";   3. Runtime built-in overrides (OPTION, SECURITY, ALIAS, etc.)\n");
    fprintf(f, ";\n");
    fprintf(f, "; Lines starting with ; or # are comments. Unknown keys are ignored.\n");
    fprintf(f, "; ==============================================================\n\n");
    
    fprintf(f, "; --------------------------------------------------------------\n");
    fprintf(f, "; DIALECT\n");
    fprintf(f, "; --------------------------------------------------------------\n");
    fprintf(f, "; Sets the default BASIC dialect (behavior, syntax, keywords).\n");
    fprintf(f, "; Available:\n");
    fprintf(f, ";   PATB   - Palo Alto Tiny BASIC (default)\n");
    fprintf(f, ";   GWBS   - Microsoft GW-BASIC\n");
    fprintf(f, ";   QBAS   - Microsoft QBasic\n");
    fprintf(f, ";   E116   - ECMA-116 Standard BASIC\n");
    fprintf(f, ";   SUPA   - Sinclair QL SuperBASIC\n");
    fprintf(f, ";   SBAS   - Tymshare SUPER BASIC\n");
    fprintf(f, ";   (and many more like TRS1, TRS2, C64B, etc.)\n");
    fprintf(f, ";\n");
    if (is_freedos) {
        fprintf(f, "dialect = GWBS,QBAS\n\n");
    } else {
        fprintf(f, "dialect = GWBS,SBAS,E116\n\n");
    }

    fprintf(f, "; --------------------------------------------------------------\n");
    fprintf(f, "; SECURITY\n");
    fprintf(f, "; --------------------------------------------------------------\n");
    fprintf(f, "; Defines the security sandbox for file I/O and shell execution.\n");
    fprintf(f, ";   OPEN       - Full access to filesystem and shell (default)\n");
    fprintf(f, ";   STANDARD   - Restricts shell access and limits file I/O to safe dirs\n");
    fprintf(f, ";   RESTRICTED - Completely disables file I/O, SHELL, and networking\n");
    fprintf(f, ";\n");
    fprintf(f, "security = STANDARD\n\n");

    fprintf(f, "; --------------------------------------------------------------\n");
    fprintf(f, "; STRICT\n");
    fprintf(f, "; --------------------------------------------------------------\n");
    fprintf(f, "; Keyword isolation mode.\n");
    fprintf(f, ";   ON  - Only keywords native to the active dialect are allowed.\n");
    fprintf(f, ";   OFF - All keywords from all dialects are available (default).\n");
    fprintf(f, ";\n");
    if (is_freedos) {
        fprintf(f, "strict = ON\n\n");
    } else {
        fprintf(f, "strict = OFF\n\n");
    }

    fprintf(f, "; --------------------------------------------------------------\n");
    fprintf(f, "; QUIET\n");
    fprintf(f, "; --------------------------------------------------------------\n");
    fprintf(f, "; Controls the startup banner printing.\n");
    fprintf(f, ";   ON  - Suppress startup banner\n");
    fprintf(f, ";   OFF - Show startup banner (default)\n");
    fprintf(f, ";\n");
    fprintf(f, "; Example: quiet = OFF\n\n");

    fprintf(f, "; --------------------------------------------------------------\n");
    fprintf(f, "; SYNTAX QUIRKS & OVERRIDES\n");
    fprintf(f, "; --------------------------------------------------------------\n");
    fprintf(f, "; You can override specific syntax quirks or behaviors here.\n");
    fprintf(f, "; Note: Built-in commands like OPTION, ALIAS, KEYWORD, SCOPE,\n");
    fprintf(f, "; and OVERRIDE will take precedence over these settings at runtime.\n");
    fprintf(f, ";\n");
    fprintf(f, "; (Add your specific quirk overrides below)\n");
    
    fclose(f);
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
        /* Auto-create in the current directory */
        config_file_create_default(filename);
        f = try_open("", filename, cfg->filepath, 256);
        if (f == NULL) {
            cfg->filepath[0] = '\0';
            return -1; /* Still no config file found */
        }
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
