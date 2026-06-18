/*
 * ---
 * BASIC++ Interpreter - ext_lib.c
 * ---
 *
 * External Library loader implementation.
 *
 * IMPLEMENTATION:
 * Static table of MAX_EXT_LIBS slots. Libraries are BASIC++
 * source files (.lib / .bas) loaded into a SEPARATE library
 * program space -- NOT the user's line-number range.
 *
 * This design avoids line-number conflicts and supports
 * QBasic-style code that does not use line numbers.
 *
 * Library files can declare metadata via REM comments:
 *   REM @LIBRARY MATHLIB
 *   REM @VERSION 1.0
 *   REM @SECURITY SAFE
 *
 * FORMAT SUPPORT (Hybrid):
 * The loader auto-detects tokenized vs source format by
 * checking for a binary header signature. Source format
 * is always supported; tokenized is platform-dependent.
 *
 * C89/C90 COMPLIANT.
 *
 * ---
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "ext_lib_api.h"
#include "lib_space.h"
#include "../security.h"

/* --- Slot --- */
static BppExtLib lib_table[MAX_EXT_LIBS];
static int lib_count = 0;

/* --- Case-insensitive compare (C89) --- */
static int lib_str_iequal(const char *a, const char *b)
{
    if (!a || !b) return 0;
    while (*a && *b) {
        if (toupper((unsigned char)*a) !=
            toupper((unsigned char)*b))
            return 0;
        a++; b++;
    }
    return (*a == '\0' && *b == '\0');
}

/* --- Parse REM @TAG VALUE from a line ---
 * Returns pointer to value after tag, or NULL.
 */
static const char *parse_rem_tag(const char *line,
                                  const char *tag)
{
    const char *p = line;
    /* Skip leading whitespace and line number */
    while (*p && (*p == ' ' || (*p >= '0' && *p <= '9')))
        p++;
    /* Check for REM */
    if (toupper((unsigned char)p[0]) != 'R' ||
        toupper((unsigned char)p[1]) != 'E' ||
        toupper((unsigned char)p[2]) != 'M') return NULL;
    p += 3;
    /* Skip space after REM */
    while (*p == ' ') p++;
    /* Check for @ */
    if (*p != '@') return NULL;
    p++;
    /* Check tag */
    while (*tag) {
        if (toupper((unsigned char)*p) !=
            toupper((unsigned char)*tag)) return NULL;
        p++; tag++;
    }
    /* Skip space after tag */
    while (*p == ' ') p++;
    return p;
}

/* --- ext_lib_init --- */
void ext_lib_init(void)
{
    memset(lib_table, 0, sizeof(lib_table));
    lib_count = 0;
}

/* --- ext_lib_load ---
 * Load a BASIC++ library file.
 *
 * Steps:
 *   1. Security check (SECOP_EXT_LOAD)
 *   2. Path validation
 *   3. Open file, scan REM @LIBRARY / @VERSION / @SECURITY
 *   4. Validate pinned level
 *   5. Store in library program space
 *   6. Register library functions in funcreg
 */
int ext_lib_load(const char *path, void *rt)
{
    FILE *fp;
    char line[256];
    int slot = -1;
    const char *val;
    BppExtLib temp;

    (void)rt;

    /* Security gate */
    if (security_check(SECOP_EXT_LOAD, 0) != 0)
        return -1;
    if (security_check_path(path, 0) != 0)
        return -1;

    /* Auto-detect .BPL binary library */
    {
        int plen = (int)strlen(path);
        if (plen >= 4 && (path[plen-4] == '.' &&
            (path[plen-3] == 'b' || path[plen-3] == 'B') &&
            (path[plen-2] == 'p' || path[plen-2] == 'P') &&
            (path[plen-1] == 'l' || path[plen-1] == 'L'))) {
            /* Load pre-compiled .BPL */
            extern int bpl_load(const char *filename,
                                void *lib);
            extern int lib_space_load_bpl(const char *path);
            return lib_space_load_bpl(path);
        }
    }

    /* Find a free slot */
    for (int i = 0; i < lib_count; i++) {
        if (!lib_table[i].loaded) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        if (lib_count >= MAX_EXT_LIBS) {
            printf("Library table full.\n");
            return -1;
        }
        slot = lib_count;
    }

    /* Initialize temp descriptor */
    memset(&temp, 0, sizeof(temp));
    strncpy(temp.path, path, 255);
    temp.required_level = SEC_COUNT; /* unpinned default */
    temp.format = EXT_LIB_SOURCE;

    /* Open and scan header lines */
    fp = fopen(path, "r");
    if (!fp) {
        printf("Cannot open library: %s\n", path);
        return -1;
    }

    /* Check for tokenized format (binary signature) */
    {
        int ch = fgetc(fp);
        if (ch == 0xBB) { /* BPP tokenized signature */
            temp.format = EXT_LIB_TOKENIZED;
            /* Tokenized loading is platform-dependent.
             * For now, only source format is implemented.
             * Tokenized support is a future enhancement. */
            printf("Tokenized library format not yet "
                   "supported: %s\n", path);
            fclose(fp);
            return -1;
        }
        rewind(fp);
    }

    /* Scan REM header tags (first 10 lines max) */
    for (int i = 0; i < 10; i++) {
        if (!fgets(line, sizeof(line), fp)) break;

        val = parse_rem_tag(line, "LIBRARY");
        if (val) {
            /* Copy library name (trim newline) */
            strncpy(temp.name, val, 63);
            temp.name[63] = '\0';
            {
                char *nl = strchr(temp.name, '\n');
                if (nl) *nl = '\0';
                nl = strchr(temp.name, '\r');
                if (nl) *nl = '\0';
                /* Trim trailing spaces */
                nl = temp.name + strlen(temp.name) - 1;
                while (nl >= temp.name && *nl == ' ')
                    *nl-- = '\0';
            }
            continue;
        }

        val = parse_rem_tag(line, "VERSION");
        if (val) {
            strncpy(temp.version, val, 15);
            temp.version[15] = '\0';
            {
                char *nl = strchr(temp.version, '\n');
                if (nl) *nl = '\0';
            }
            continue;
        }

        val = parse_rem_tag(line, "SECURITY");
        if (val) {
            char lvl_name[32];
            int lvl;
            strncpy(lvl_name, val, 31);
            lvl_name[31] = '\0';
            {
                char *nl = strchr(lvl_name, '\n');
                if (nl) *nl = '\0';
                nl = strchr(lvl_name, '\r');
                if (nl) *nl = '\0';
                nl = lvl_name + strlen(lvl_name) - 1;
                while (nl >= lvl_name && *nl == ' ')
                    *nl-- = '\0';
            }
            lvl = security_find_level_by_name(lvl_name);
            if (lvl >= 0) {
                temp.required_level = (SecLevel)lvl;
            }
            continue;
        }
    }

    fclose(fp);

    /* If no name found, derive from filename */
    if (temp.name[0] == '\0') {
        const char *base = path;
        const char *p2 = path;
        while (*p2) {
            if (*p2 == '/' || *p2 == '\\')
                base = p2 + 1;
            p2++;
        }
        strncpy(temp.name, base, 63);
        temp.name[63] = '\0';
        /* Remove extension */
        {
            char *dot = strrchr(temp.name, '.');
            if (dot) *dot = '\0';
        }
        /* Uppercase */
        for (int i = 0; temp.name[i]; i++)
            temp.name[i] = (char)toupper(
                (unsigned char)temp.name[i]);
    }

    /* Check security pinning */
    if (!security_check_pinned_level(
            temp.required_level)) {
        printf("Library '%s' requires security level "
               "%s (current: %s).\n",
               temp.name,
               security_level_name(temp.required_level),
               security_level_name(
                   security_get_level()));
        return -1;
    }

    /* Store in table */
    temp.loaded = 1;
    lib_table[slot] = temp;
    if (slot >= lib_count) lib_count = slot + 1;

    /* Delegate to lib_space for full compilation.
     * lib_space handles:
     *   - Pass 1: symbol scanning (SUB/FUNCTION/DEF FN)
     *   - Pass 2: block parser -> AST -> PCode emission
     *   - Symbol entry point patching
     *   - Variable space initialization
     */
    {
        int ls_slot = lib_space_load_source(path,
                          LIB_EXT_LIBRARY, rt);
        if (ls_slot < 0) {
            printf("Warning: metadata loaded but "
                   "compilation failed for '%s'.\n",
                   temp.name);
        } else {
            /* Update func_count from lib_space symbols */
            LoadedLibrary *ls = lib_space_find(temp.name);
            if (ls) {
                lib_table[slot].func_count =
                    ls->symbol_count;
            }
        }
    }

    return 0;
}

/* --- ext_lib_unload --- */
int ext_lib_unload(const char *name)
{
    for (int i = 0; i < lib_count; i++) {
        if (lib_table[i].loaded &&
            lib_str_iequal(lib_table[i].name, name)) {
            /* Also unload from lib_space (frees bytecode) */
            lib_space_unload(name);
            lib_table[i].loaded = 0;
            printf("Unloaded library: %s\n", name);
            return 0;
        }
    }
    /* Fallback: try lib_space directly (BPL-loaded libs
     * bypass lib_table and go straight to lib_space) */
    if (lib_space_unload(name) == 0) {
        printf("Unloaded library: %s\n", name);
        return 0;
    }
    printf("Library '%s' not found.\n", name);
    return -1;
}

/* --- ext_lib_is_loaded --- */
int ext_lib_is_loaded(const char *name)
{
    for (int i = 0; i < lib_count; i++) {
        if (lib_table[i].loaded &&
            lib_str_iequal(lib_table[i].name, name))
            return 1;
    }
    return 0;
}

/* --- ext_lib_find --- */
const BppExtLib *ext_lib_find(const char *name)
{
    for (int i = 0; i < lib_count; i++) {
        if (lib_table[i].loaded &&
            lib_str_iequal(lib_table[i].name, name))
            return &lib_table[i];
    }
    return NULL;
}

/* --- ext_lib_list --- */
void ext_lib_list(void)
{
    int found = 0;
    printf("--- Loaded Libraries ---\n");
    for (int i = 0; i < lib_count; i++) {
        if (lib_table[i].loaded) {
            printf("  %s v%s [%s] (%s)\n",
                lib_table[i].name,
                lib_table[i].version,
                lib_table[i].required_level != SEC_COUNT
                    ? security_level_name(
                          lib_table[i].required_level)
                    : "any",
                lib_table[i].format == EXT_LIB_TOKENIZED
                    ? "tokenized" : "source");
            found++;
        }
    }
    if (!found) printf("  (none)\n");
}

/* --- ext_lib_count --- */
int ext_lib_count(void)
{
    int n = 0;
    for (int i = 0; i < lib_count; i++) {
        if (lib_table[i].loaded) n++;
    }
    return n;
}
