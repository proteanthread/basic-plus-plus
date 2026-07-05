/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: security_core.c
 * Subsystem: Capabilities Sandbox Access Guard
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Enforces security sandbox levels (OPEN, RESTRICTED, etc.).
 *
 * 2. WHAT TO EXPECT:
 *    Validates system commands before execution.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Sandbox capability flags.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Sandbox policy matrix rules.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If access is blocked, check security settings.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE SECURITY SANDBOX GATES
 * File: security_core.c
 * ===================================================================== */

#include "security_core.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static SecLevel current_level = SEC_OPEN;

static int restrict_ops[SECOP_COUNT];
static int restrict_ops_count = 0;

#define MAX_RESTRICT_KEYWORDS 64
static int restrict_kw_ids[MAX_RESTRICT_KEYWORDS];
static int restrict_kw_count = 0;

static const int allowed[SEC_COUNT][SECOP_COUNT] = {
    /* SEC_OPEN (0): all operations permitted */
    {   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },

    /* SEC_SAFE (1): secure but functional */
    {   1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 },

    /* SEC_STANDARD (2): controlled sandbox */
    {   1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1 },

    /* SEC_EDUCATIONAL (3): classroom mode */
    {   1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },

    /* SEC_RESTRICTED (4): very limited */
    {   1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },

    /* SEC_PARANOID (5): pure computation only */
    {   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 }
};

static const char *op_names[SECOP_COUNT] = {
    "file read",
    "file write",
    "file management",
    "block I/O",
    "stream I/O",
    "compile",
    "chain",
    "system",
    "module activation",
    "usb access",
    "virtual device access",
    "terminal intercept",
    "console hijack",
    "dynamic string evaluation",
    "network sockets",
    "memory read (PEEK)",
    "memory write (POKE)",
    "program management (LIST/LOAD/SAVE/RUN)",
    "external code loading (LOAD FUNCTION/LIBRARY/MODULE)"
};

static const char *level_names[SEC_COUNT] = {
    "OPEN",
    "SAFE",
    "STANDARD",
    "EDUCATIONAL",
    "RESTRICTED",
    "PARANOID"
};

void security_core_init(SecLevel level)
{
    if (level >= 0 && level < SEC_COUNT) {
        current_level = level;
    } else {
        current_level = SEC_OPEN;
    }
    memset(restrict_ops, 0, sizeof(restrict_ops));
    restrict_ops_count = 0;
    restrict_kw_count = 0;
}

SecLevel security_core_get_level(void)
{
    return current_level;
}

void security_core_set_level(SecLevel level)
{
    if (level >= 0 && level < SEC_COUNT) {
        current_level = level;
    }
}

const char *security_core_level_name(SecLevel level)
{
    if (level >= 0 && level < SEC_COUNT) {
        return level_names[level];
    }
    return "UNKNOWN";
}

int security_core_find_level_by_name(const char *name)
{
    int i;
    if (!name) return -1;

    for (i = 0; i < SEC_COUNT; i++) {
        const char *a = name;
        const char *b = level_names[i];
        int match = 1;

        while (*a && *b) {
            if (toupper((unsigned char)*a) != toupper((unsigned char)*b)) {
                match = 0;
                break;
            }
            a++;
            b++;
        }
        if (match && *a == '\0' && *b == '\0') {
            return i;
        }
    }
    return -1;
}

int security_core_check_op(SecLevel level, SecOperation op)
{
    if (op < 0 || op >= SECOP_COUNT) return -1;
    if (restrict_ops[op]) return -1;
    if (level == SEC_OPEN) return 0;
    if (allowed[level][op]) return 0;
    return -1;
}

int security_core_check_path(SecLevel level, const char *path)
{
    const char *p;
    const char *dot;
    int path_len;

    if (level == SEC_OPEN) return 0;

    if (!path || path[0] == '\0') {
        return -1;
    }

    if (path[0] == '/' || path[0] == '\\') {
        return -1;
    }
    path_len = (int)strlen(path);
    if (path_len >= 2 && path[1] == ':') {
        return -1;
    }

    p = path;
    while (*p) {
        if (p[0] == '.' && p[1] == '.') {
            return -1;
        }
        p++;
    }

    dot = NULL;
    p = path;
    while (*p) {
        if (*p == '.') dot = p;
        p++;
    }

    if (dot) {
        int i;
        const char *exts[] = { ".dll", ".so", ".lib", ".bpl",
                               ".bpp", ".bas", ".spec", ".yaml" };
        for (i = 0; i < 8; i++) {
            int elen = (int)strlen(exts[i]);
            int dlen = (int)strlen(dot);
            if (dlen == elen) {
                int j, match = 1;
                for (j = 0; j < elen; j++) {
                    if (toupper((unsigned char)dot[j]) != toupper((unsigned char)exts[i][j])) {
                        match = 0; break;
                    }
                }
                if (match) return 0;
            }
        }
    }

    return -1;
}

int security_core_check_port(SecLevel level, int port)
{
    if (level == SEC_OPEN) return 0;

    switch (level) {
    case SEC_EDUCATIONAL:
    case SEC_RESTRICTED:
    case SEC_PARANOID:
        return -1;
    default:
        break;
    }

    if (port >= 1024 && port <= 49151) return 0;

    switch (port) {
    case 21:    case 22:    case 23:    case 25:
    case 53:    case 80:    case 110:   case 119:
    case 143:   case 161:   case 162:   case 443:
    case 465:   case 587:   case 993:   case 995:
    case 6667:  case 6697:  case 8080:  case 8443:
    case 16384:
        return 0;
    default:
        break;
    }

    return -1;
}

int security_core_restrict_op(SecOperation op, int restricted)
{
    if (op < 0 || op >= SECOP_COUNT) return -1;
    if (restricted) {
        if (!restrict_ops[op]) {
            restrict_ops[op] = 1;
            restrict_ops_count++;
        }
    } else {
        if (restrict_ops[op]) {
            restrict_ops[op] = 0;
            restrict_ops_count--;
        }
    }
    return 0;
}

int security_core_is_op_restricted(SecOperation op)
{
    if (op < 0 || op >= SECOP_COUNT) return 0;
    return restrict_ops[op];
}

int security_core_restrict_keyword(int kw_id, int restricted)
{
    int i;
    if (restricted) {
        for (i = 0; i < restrict_kw_count; i++) {
            if (restrict_kw_ids[i] == kw_id) return 0;
        }
        if (restrict_kw_count >= MAX_RESTRICT_KEYWORDS) return -1;
        restrict_kw_ids[restrict_kw_count++] = kw_id;
    } else {
        for (i = 0; i < restrict_kw_count; i++) {
            if (restrict_kw_ids[i] == kw_id) {
                restrict_kw_ids[i] = restrict_kw_ids[--restrict_kw_count];
                return 0;
            }
        }
    }
    return 0;
}

int security_core_is_keyword_restricted(int kw_id)
{
    int i;
    for (i = 0; i < restrict_kw_count; i++) {
        if (restrict_kw_ids[i] == kw_id) return 1;
    }
    return 0;
}

int security_core_restrict_count(void)
{
    return restrict_ops_count + restrict_kw_count;
}

void security_core_restrict_clear(void)
{
    memset(restrict_ops, 0, sizeof(restrict_ops));
    restrict_ops_count = 0;
    restrict_kw_count = 0;
}
