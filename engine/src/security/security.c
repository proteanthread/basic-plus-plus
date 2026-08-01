/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file security.c
 * @brief Security sandbox and restriction registry implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements the 6x19 security permission matrix, path validation, memory checks,
 *   port check rules, and runtime restrict operations.
 * - Why it exists: Protects host resources (files, shell, network, memory) from unauthorized access.
 * - Why it works this way: It defines static tables of permissions. The check functions evaluate
 *   active security level allowances and explicit keyword/operation restrictions, printing errors on failure.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Allowed matrix mappings, whitelisted network ports, whitelisted file extensions.
 * - What cannot be changed: One-way ratchet constraint (security level can only be raised, never lowered).
 * - What to expect: security_check returns 0 if allowed, -1 if blocked.
 * - What to do if something breaks: Check active restrict counts and match level names.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Relies on platform module for memory segment capabilities.
 * - Portability concerns: Standard C17.
 */

#ifndef BPP_LITE_BUILD

#include "security/security.h"
#include "module/module.h"
#include "platform/platform.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static BppSecLevel current_level = SEC_OPEN;

/* Override restrictions per operation */
static int restrict_ops[SECOP_COUNT];
static int restrict_ops_count = 0;

/* Blacklisted keyword IDs */
#define MAX_RESTRICT_KEYWORDS 64
static int restrict_kw_ids[MAX_RESTRICT_KEYWORDS];
static int restrict_kw_count = 0;

/* Permission Matrix: allowed[level][operation] */
static const int allowed[SEC_COUNT][SECOP_COUNT] = {
    /* SEC_OPEN (0) */
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    /* SEC_SAFE (1) */
    { 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    /* SEC_STANDARD (2) */
    { 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1 },
    /* SEC_EDUCATIONAL (3) */
    { 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },
    /* SEC_RESTRICTED (4) */
    { 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },
    /* SEC_PARANOID (5) */
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 }
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
    "external code loading"
};

static const char *level_names[SEC_COUNT] = {
    "OPEN",
    "SAFE",
    "STANDARD",
    "EDUCATIONAL",
    "RESTRICTED",
    "PARANOID"
};

void security_init(BppSecLevel level) {
    if (level >= 0 && level < SEC_COUNT) {
        current_level = level;
    } else {
        current_level = SEC_OPEN;
    }
    memset(restrict_ops, 0, sizeof(restrict_ops));
    restrict_ops_count = 0;
    restrict_kw_count = 0;
}

BppSecLevel security_get_level(void) {
    return current_level;
}

void security_set_level(BppSecLevel level) {
    if (level >= 0 && level < SEC_COUNT) {
        current_level = level;
    }
}

const char *security_level_name(BppSecLevel level) {
    if (level >= 0 && level < SEC_COUNT) {
        return level_names[level];
    }
    return "UNKNOWN";
}

int security_find_level_by_name(const char *name) {
    if (!name) return -1;
    for (int i = 0; i < SEC_COUNT; ++i) {
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

int security_check(BppSecOperation op, int line_num) {
    if (op < 0 || op >= SECOP_COUNT) return -1;

    if (restrict_ops[op]) {
        printf("?Error: %s restricted via SECURITY RESTRICT", op_names[op]);
        if (line_num > 0) printf(" in line %d", line_num);
        printf("\n");
        return -1;
    }

    if (current_level == SEC_OPEN) return 0;

    if (allowed[current_level][op]) {
        return 0;
    }

    printf("?Error: %s not permitted at level %s", op_names[op], level_names[current_level]);
    if (line_num > 0) printf(" in line %d", line_num);
    printf("\n");
    return -1;
}

int security_module_allowed(unsigned int capabilities) {
    if (current_level == SEC_OPEN) return 1;
    if (current_level == SEC_PARANOID) return 0;

    if (current_level == SEC_RESTRICTED || current_level == SEC_EDUCATIONAL) {
        if (capabilities & (CAP_IO | CAP_FILE | CAP_SYSTEM | CAP_GRAPHICS | CAP_SOUND | CAP_NETWORK | CAP_USB)) {
            return 0;
        }
        return 1;
    }

    if (current_level == SEC_STANDARD || current_level == SEC_SAFE) {
        if (capabilities & (CAP_SYSTEM | CAP_USB)) {
            return 0;
        }
        return 1;
    }

    return 1;
}

int security_check_pinned_level(BppSecLevel required_level) {
    if (required_level == SEC_COUNT) return 1;
    if (current_level == SEC_PARANOID) return 0;
    return ((int)current_level <= (int)required_level) ? 1 : 0;
}

int security_check_mem(unsigned long address, int size) {
    (void)size;
    if (current_level == SEC_OPEN) return 0;

    /* emulated low memory check (< 64K) on modern architectures */
    if ((current_level == SEC_SAFE || current_level == SEC_STANDARD) && address < 0x10000) {
        return 0;
    }

    printf("?Error: memory access at 0x%lX not permitted at level %s\n", address, level_names[current_level]);
    return -1;
}

int security_check_port(int port, int line_num) {
    if (current_level == SEC_OPEN) return 0;

    switch (current_level) {
        case SEC_EDUCATIONAL:
        case SEC_RESTRICTED:
        case SEC_PARANOID:
            printf("?Error: network access not permitted at level %s", level_names[current_level]);
            if (line_num > 0) printf(" in line %d", line_num);
            printf("\n");
            return -1;
        default:
            break;
    }

    if (port >= 1024 && port <= 49151) return 0;

    switch (port) {
        case 21:  case 22:  case 23:  case 25:
        case 53:  case 80:  case 110: case 119:
        case 143: case 161: case 162: case 443:
        case 465: case 587: case 993: case 995:
        case 8080: case 8443:
            return 0;
        default:
            break;
    }

    printf("?Error: port %d not permitted at level %s", port, level_names[current_level]);
    if (line_num > 0) printf(" in line %d", line_num);
    printf("\n");
    return -1;
}

int security_check_path(const char *path, int line_num) {
    if (!path || path[0] == '\0') {
        printf("?Error: empty path not permitted");
        if (line_num > 0) printf(" in line %d", line_num);
        printf("\n");
        return -1;
    }

    if (path[0] == '/' || path[0] == '\\') {
        printf("?Error: absolute path '%s' not permitted", path);
        if (line_num > 0) printf(" in line %d", line_num);
        printf("\n");
        return -1;
    }

    size_t path_len = strlen(path);
    if (path_len >= 2 && path[1] == ':') {
        printf("?Error: absolute path '%s' not permitted", path);
        if (line_num > 0) printf(" in line %d", line_num);
        printf("\n");
        return -1;
    }

    const char *p = path;
    while (*p) {
        if (p[0] == '.' && p[1] == '.') {
            printf("?Error: path traversal (..) not permitted in '%s'", path);
            if (line_num > 0) printf(" in line %d", line_num);
            printf("\n");
            return -1;
        }
        p++;
    }

    const char *dot = NULL;
    p = path;
    while (*p) {
        if (*p == '.') dot = p;
        p++;
    }

    if (dot) {
        const char *exts[] = { ".dll", ".so", ".lib", ".bpl", ".bpp", ".bas", ".spec", ".yaml" };
        for (int i = 0; i < 8; ++i) {
            if (strcmp(dot, exts[i]) == 0) return 0;
        }
    }

    printf("?Error: file extension not allowed for '%s'", path);
    if (line_num > 0) printf(" in line %d", line_num);
    printf("\n");
    return -1;
}

int security_check_file_path(const char *path, int line_num) {
    if (current_level == SEC_OPEN) return 0;

    if (!path || path[0] == '\0') {
        printf("?Error: empty path not permitted");
        if (line_num > 0) printf(" in line %d", line_num);
        printf("\n");
        return -1;
    }

    if (path[0] == '/' || path[0] == '\\') {
        printf("?Error: absolute path '%s' not permitted", path);
        if (line_num > 0) printf(" in line %d", line_num);
        printf("\n");
        return -1;
    }

    size_t path_len = strlen(path);
    if (path_len >= 2 && path[1] == ':') {
        printf("?Error: absolute path '%s' not permitted", path);
        if (line_num > 0) printf(" in line %d", line_num);
        printf("\n");
        return -1;
    }

    const char *p = path;
    while (*p) {
        if (p[0] == '.' && p[1] == '.') {
            printf("?Error: path traversal (..) not permitted in '%s'", path);
            if (line_num > 0) printf(" in line %d", line_num);
            printf("\n");
            return -1;
        }
        p++;
    }

    return 0;
}


int security_restrict_op(BppSecOperation op) {
    if (op < 0 || op >= SECOP_COUNT) return -1;
    if (!restrict_ops[op]) {
        restrict_ops[op] = 1;
        restrict_ops_count++;
    }
    return 0;
}

int security_is_op_restricted(BppSecOperation op) {
    if (op < 0 || op >= SECOP_COUNT) return 0;
    return restrict_ops[op];
}

int security_restrict_keyword(int kw_id) {
    for (int i = 0; i < restrict_kw_count; ++i) {
        if (restrict_kw_ids[i] == kw_id) return 0;
    }
    if (restrict_kw_count >= MAX_RESTRICT_KEYWORDS) return -1;
    restrict_kw_ids[restrict_kw_count++] = kw_id;
    return 0;
}

int security_is_keyword_restricted(int kw_id) {
    for (int i = 0; i < restrict_kw_count; ++i) {
        if (restrict_kw_ids[i] == kw_id) return 1;
    }
    return 0;
}

void security_restrict_list(void) {
    int found = 0;
    printf("SECURITY RESTRICT overrides:\n");
    for (int i = 0; i < SECOP_COUNT; ++i) {
        if (restrict_ops[i]) {
            printf("  %-8s %s\n", "OP", op_names[i]);
            found++;
        }
    }
    for (int i = 0; i < restrict_kw_count; ++i) {
        printf("  %-8s keyword ID %d\n", "KEYWORD", restrict_kw_ids[i]);
        found++;
    }
    if (!found) {
        printf("  (no restrictions)\n");
    }
}

int security_restrict_count(void) {
    return restrict_ops_count + restrict_kw_count;
}

#endif /* BPP_LITE_BUILD */
