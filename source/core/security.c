/*
 * ---
 * BASIC++ Interpreter - security.c
 * ---
 *
 * Security system implementation.
 *
 * 6-LEVEL SECURITY MODEL with 19 operations.
 * See security.h for the full specification.
 *
 * ---
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "security.h"
#include "module.h"
#include "platform.h"

/* --- State --- */
static SecLevel current_level = SEC_OPEN;

/* --- SECURITY RESTRICT overrides --- */
static int restrict_ops[SECOP_COUNT];
static int restrict_ops_count = 0;

#define MAX_RESTRICT_KEYWORDS 64
static int restrict_kw_ids[MAX_RESTRICT_KEYWORDS];
static int restrict_kw_count = 0;

/* --- Permission Matrix ---
 * allowed[level][operation] - 1 = permitted, 0 = denied
 *
 * Index mapping:
 *  0=FILE_READ  1=FILE_WRITE 2=FILE_MGMT  3=FILE_BLOCK
 *  4=FILE_STRM  5=COMPILE    6=CHAIN      7=SYSTEM
 *  8=MODULE     9=USB       10=VDEV      11=VTERM
 * 12=VCON      13=EVAL      14=NETWORK   15=MEM_READ
 * 16=MEM_WRITE 17=PROG_MGMT 18=EXT_LOAD
 *
 * Level order: OPEN(0), SAFE(1), STANDARD(2),
 *              EDUCATIONAL(3), RESTRICTED(4), PARANOID(5)
 */
static const int allowed[SEC_COUNT][SECOP_COUNT] = {
    /* SEC_OPEN (0): all operations permitted */
    /*  RD WR MG BK ST CO CH SY MO US VD VT VC EV NW MR MW PM EL */
    {   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },

    /* SEC_SAFE (1): secure but functional */
    /*  RD WR MG BK ST CO CH SY MO US VD VT VC EV NW MR MW PM EL */
    {   1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 },

    /* SEC_STANDARD (2): controlled sandbox */
    /*  RD WR MG BK ST CO CH SY MO US VD VT VC EV NW MR MW PM EL */
    {   1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1 },

    /* SEC_EDUCATIONAL (3): classroom mode */
    /*  RD WR MG BK ST CO CH SY MO US VD VT VC EV NW MR MW PM EL */
    {   1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },

    /* SEC_RESTRICTED (4): very limited */
    /*  RD WR MG BK ST CO CH SY MO US VD VT VC EV NW MR MW PM EL */
    {   1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },

    /* SEC_PARANOID (5): pure computation only */
    /*  RD WR MG BK ST CO CH SY MO US VD VT VC EV NW MR MW PM EL */
    {   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 }
};

/* --- Operation names (for error messages) --- */
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

/* --- Level names --- */
static const char *level_names[SEC_COUNT] = {
    "OPEN",
    "SAFE",
    "STANDARD",
    "EDUCATIONAL",
    "RESTRICTED",
    "PARANOID"
};

/* --- security_init --- */
void security_init(SecLevel level)
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

/* --- security_get_level --- */
SecLevel security_get_level(void)
{
 return current_level;
}

/* --- security_set_level --- */
void security_set_level(SecLevel level)
{
 if (level >= 0 && level < SEC_COUNT) {
 current_level = level;
 }
}

/* --- security_level_name --- */
const char *security_level_name(SecLevel level)
{
 if (level >= 0 && level < SEC_COUNT) {
 return level_names[level];
 }
 return "UNKNOWN";
}

/* --- security_find_level_by_name --- */
int security_find_level_by_name(const char *name)
{
    int i;
    if (!name) return -1;

    for (i = 0; i < SEC_COUNT; i++) {
        const char *a = name;
        const char *b = level_names[i];
        int match = 1;

        while (*a && *b) {
            if (toupper((unsigned char)*a) !=
                toupper((unsigned char)*b)) {
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

/* --- security_check --- */
int security_check(SecOperation op, int line_num)
{
 if (op < 0 || op >= SECOP_COUNT) return -1;

 /* Check RESTRICT overrides first */
 if (restrict_ops[op]) {
  printf("SORRY? Security: %s restricted "
   "via SECURITY RESTRICT",
   op_names[op]);
  if (line_num > 0) printf(" in line %d", line_num);
  printf("\n");
  return -1;
 }

 /* Fast path: SEC_OPEN permits everything */
 if (current_level == SEC_OPEN) return 0;

 if (allowed[current_level][op]) {
 return 0;
 }

 /* Denied */
 printf("SORRY? Security: %s not permitted "
 "at level %s",
 op_names[op], level_names[current_level]);
 if (line_num > 0) {
 printf(" in line %d", line_num);
 }
 printf("\n");
 return -1;
}

/* --- security_module_allowed --- */
int security_module_allowed(unsigned int capabilities)
{
    if (current_level == SEC_OPEN) return 1;

    if (current_level == SEC_PARANOID) return 0;

    if (current_level == SEC_RESTRICTED ||
        current_level == SEC_EDUCATIONAL) {
        if (capabilities & (CAP_IO | CAP_FILE | CAP_SYSTEM |
                            CAP_GRAPHICS | CAP_SOUND |
                            CAP_NETWORK | CAP_USB |
                            CAP_GPIO | CAP_I2C | CAP_SPI |
                            CAP_SENSOR | CAP_CAMERA |
                            CAP_BLUETOOTH)) {
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

/* --- security_check_pinned_level --- */
int security_check_pinned_level(SecLevel required_level)
{
    if (required_level == SEC_COUNT) return 1;
    if (current_level == SEC_PARANOID) return 0;
    return ((int)current_level >= (int)required_level) ? 1 : 0;
}

/* --- security_check_mem --- */
int security_check_mem(unsigned long address, int size)
{
    const PlatformInfo *plat;

    (void)size;

    if (current_level == SEC_OPEN) return 0;

    plat = platform_get_info();
    if (plat->id == PLAT_DOS) {
        if (current_level == SEC_SAFE ||
            current_level == SEC_STANDARD) return 0;
    } else {
        if ((current_level == SEC_SAFE ||
             current_level == SEC_STANDARD) &&
            address < 0x10000) {
            return 0;
        }
    }

    printf("SORRY? Security: memory access at 0x%lX not permitted at level %s\n",
           address, level_names[current_level]);
    return -1;
}

/* --- security_check_port --- */
int security_check_port(int port, int line_num)
{
    if (current_level == SEC_OPEN) return 0;

    switch (current_level) {
    case SEC_EDUCATIONAL:
    case SEC_RESTRICTED:
    case SEC_PARANOID:
        printf("SORRY? Security: network access "
               "not permitted at level %s",
               level_names[current_level]);
        if (line_num > 0) printf(" in line %d", line_num);
        printf("\n");
        return -1;
    default:
        break;
    }

    /* SEC_SAFE / SEC_STANDARD: well-known + ephemeral ports */
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

    printf("SORRY? Security: port %d "
           "not permitted at level %s",
           port, level_names[current_level]);
    if (line_num > 0) printf(" in line %d", line_num);
    printf("\n");
    return -1;
}

/* --- security_check_path --- */
int security_check_path(const char *path, int line_num)
{
    const char *p;
    const char *dot;
    int path_len;

    if (!path || path[0] == '\0') {
        printf("SORRY? Security: empty path not permitted");
        if (line_num > 0) printf(" in line %d", line_num);
        printf("\n");
        return -1;
    }

    /* Reject absolute paths */
    if (path[0] == '/' || path[0] == '\\') {
        printf("SORRY? Security: absolute path '%s' not permitted", path);
        if (line_num > 0) printf(" in line %d", line_num);
        printf("\n");
        return -1;
    }
    path_len = (int)strlen(path);
    if (path_len >= 2 && path[1] == ':') {
        printf("SORRY? Security: absolute path '%s' not permitted", path);
        if (line_num > 0) printf(" in line %d", line_num);
        printf("\n");
        return -1;
    }

    /* Reject path traversal (..) */
    p = path;
    while (*p) {
        if (p[0] == '.' && p[1] == '.') {
            printf("SORRY? Security: path traversal (..) not permitted in '%s'", path);
            if (line_num > 0) printf(" in line %d", line_num);
            printf("\n");
            return -1;
        }
        p++;
    }

    /* Check extension whitelist */
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
                    if (toupper((unsigned char)dot[j]) !=
                        toupper((unsigned char)exts[i][j])) {
                        match = 0; break;
                    }
                }
                if (match) return 0;
            }
        }
    }

    printf("SORRY? Security: file extension not allowed for '%s'", path);
    if (line_num > 0) printf(" in line %d", line_num);
    printf("\n");
    return -1;
}

/* --- SECURITY RESTRICT API --- */

int security_restrict_op(SecOperation op)
{
    if (op < 0 || op >= SECOP_COUNT) return -1;
    if (!restrict_ops[op]) {
        restrict_ops[op] = 1;
        restrict_ops_count++;
    }
    return 0;
}

int security_is_op_restricted(SecOperation op)
{
    if (op < 0 || op >= SECOP_COUNT) return 0;
    return restrict_ops[op];
}

int security_restrict_keyword(int kw_id)
{
    int i;
    for (i = 0; i < restrict_kw_count; i++) {
        if (restrict_kw_ids[i] == kw_id) return 0;
    }
    if (restrict_kw_count >= MAX_RESTRICT_KEYWORDS) return -1;
    restrict_kw_ids[restrict_kw_count++] = kw_id;
    return 0;
}

int security_is_keyword_restricted(int kw_id)
{
    int i;
    for (i = 0; i < restrict_kw_count; i++) {
        if (restrict_kw_ids[i] == kw_id) return 1;
    }
    return 0;
}

void security_restrict_list(void)
{
    int i, found = 0;
    printf("SECURITY RESTRICT overrides:\n");
    for (i = 0; i < SECOP_COUNT; i++) {
        if (restrict_ops[i]) {
            printf("  %-8s %s\n", "OP", op_names[i]);
            found++;
        }
    }
    for (i = 0; i < restrict_kw_count; i++) {
        printf("  %-8s keyword ID %d\n", "KEYWORD",
               restrict_kw_ids[i]);
        found++;
    }
    if (!found) {
        printf("  (no restrictions)\n");
    }
}

int security_restrict_count(void)
{
    return restrict_ops_count + restrict_kw_count;
}
