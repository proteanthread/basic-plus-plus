/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: device_alias_core.c
 * Subsystem: Pluggable Hardware Device Name Translator
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Translates legacy device identifiers (LPT1, CAS:) to virtual devices.
 *
 * 2. WHAT TO EXPECT:
 *    Resolves device mappings dynamically based on current dialect.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Device mapping lists, prefix matching rules.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Virtual device interface signatures.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If I/O redirection fails, check device spelling and case.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE DEVICE ALIAS CORE
 * File: device_alias_core.c
 * ===================================================================== */

#include <string.h>
#include "device_alias_core.h"

// --- Spanish, case-insensitive helper compare ---
static int da_str_eq_ci(const char *a, const char *b)
{
    while (*a && *b) {
        char ca = *a, cb = *b;
        if (ca >= 'a' && ca <= 'z') ca = (char)(ca - 32);
        if (cb >= 'a' && cb <= 'z') cb = (char)(cb - 32);
        if (ca != cb) return 0;
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

static void da_copy_upper(char *dst, const char *src, int max)
{
    int i;
    for (i = 0; i < max - 1 && src[i] != '\0'; i++) {
        dst[i] = src[i];
        if (dst[i] >= 'a' && dst[i] <= 'z') {
            dst[i] = (char)(dst[i] - 32);
        }
    }
    dst[i] = '\0';
}

void device_alias_core_init(DeviceAlias *table, int *count, int max_aliases)
{
    if (table && count) {
        *count = 0;
        memset(table, 0, (size_t)max_aliases * sizeof(DeviceAlias));
    }
}

int device_alias_core_set(DeviceAlias *table, int *count, int max_aliases, const char *alias, const char *target, int direction, int dialect)
{
    int i;
    if (!table || !count || !alias || !target) {
        return -1;
    }

    // Check for existing alias (update in-place)
    for (i = 0; i < *count; i++) {
        if (da_str_eq_ci(table[i].alias, alias)) {
            da_copy_upper(table[i].target, target, sizeof(table[i].target));
            table[i].direction = direction;
            table[i].dialect = dialect;
            table[i].active = 1;
            return 0;
        }
    }

    if (*count >= max_aliases) {
        return -1;
    }

    da_copy_upper(table[*count].alias, alias, sizeof(table[*count].alias));
    da_copy_upper(table[*count].target, target, sizeof(table[*count].target));
    table[*count].direction = direction;
    table[*count].dialect = dialect;
    table[*count].active = 1;
    (*count)++;
    return 0;
}

const DeviceAlias *device_alias_core_resolve(const DeviceAlias *table, int count, const char *name)
{
    int i;
    if (!table || !name) {
        return NULL;
    }

    for (i = 0; i < count; i++) {
        if (table[i].active && da_str_eq_ci(table[i].alias, name)) {
            return &table[i];
        }
    }
    return NULL;
}

int device_alias_core_remove(DeviceAlias *table, int *count, const char *alias)
{
    int i;
    if (!table || !count || !alias) {
        return -1;
    }

    for (i = 0; i < *count; i++) {
        if (da_str_eq_ci(table[i].alias, alias)) {
            int j;
            for (j = i; j < *count - 1; j++) {
                table[j] = table[j + 1];
            }
            (*count)--;
            return 0;
        }
    }
    return -1;
}

int device_alias_core_set_active(DeviceAlias *table, int count, const char *alias, int active)
{
    int i;
    if (!table || !alias) {
        return -1;
    }

    for (i = 0; i < count; i++) {
        if (da_str_eq_ci(table[i].alias, alias)) {
            table[i].active = active;
            return 0;
        }
    }
    return -1;
}

// Preset Definitions
#if 0
static const AliasEntry atari_aliases[] = {
    { "E:",   "CON:",  DEVALIAS_BOTH   },
    { "S:",   "CON:",  DEVALIAS_OUTPUT },
    { "K:",   "CON:",  DEVALIAS_INPUT  },
    { "P:",   "ERR:",  DEVALIAS_OUTPUT },
    { "D:",   "FILE:", DEVALIAS_BOTH   },
    { "D1:",  "FILE:", DEVALIAS_BOTH   },
    { "D2:",  "FILE:", DEVALIAS_BOTH   },
    { "C:",   "FILE:", DEVALIAS_BOTH   },
    { "R:",   "SER:",  DEVALIAS_BOTH   },
    { "R1:",  "SER:",  DEVALIAS_BOTH   }
};

static const AliasEntry c64_aliases[] = {
    { "DEV0:",  "CON:",  DEVALIAS_INPUT  },
    { "DEV1:",  "FILE:", DEVALIAS_BOTH   },
    { "DEV2:",  "SER:",  DEVALIAS_BOTH   },
    { "DEV3:",  "CON:",  DEVALIAS_OUTPUT },
    { "DEV4:",  "ERR:",  DEVALIAS_OUTPUT },
    { "DEV5:",  "ERR:",  DEVALIAS_OUTPUT },
    { "DEV8:",  "FILE:", DEVALIAS_BOTH   },
    { "DEV9:",  "FILE:", DEVALIAS_BOTH   },
    { "DEV10:", "FILE:", DEVALIAS_BOTH   },
    { "DEV11:", "FILE:", DEVALIAS_BOTH   }
};

static const AliasEntry coco_aliases[] = {
    { "CAS:",  "FILE:", DEVALIAS_BOTH   },
    { "LPT:",  "ERR:",  DEVALIAS_OUTPUT },
    { "SCR:",  "CON:",  DEVALIAS_BOTH   }
};

static const AliasEntry sinclair_aliases[] = {
    { "LOWER:", "CON:",  DEVALIAS_OUTPUT },
    { "UPPER:", "CON:",  DEVALIAS_BOTH   },
    { "ZXPRN:", "ERR:",  DEVALIAS_OUTPUT },
    { "ZXNET:", "NET:",  DEVALIAS_BOTH   }
};
#endif

static const AliasEntry gwbasic_aliases[] = {
    { "SCRN:", "CON:",  DEVALIAS_OUTPUT },
    { "KYBD:", "CON:",  DEVALIAS_INPUT  },
    { "LPT1:", "ERR:",  DEVALIAS_OUTPUT },
    { "LPT2:", "ERR:",  DEVALIAS_OUTPUT },
    { "COM1:", "SER:",  DEVALIAS_BOTH   },
    { "COM2:", "SER:",  DEVALIAS_BOTH   },
    { "CAS1:", "FILE:", DEVALIAS_BOTH   }
};

static const AliasEntry qbasic_aliases[] = {
    { "CONS:", "CON:",  DEVALIAS_BOTH   },
    { "SCRN:", "CON:",  DEVALIAS_OUTPUT },
    { "LPT1:", "ERR:",  DEVALIAS_OUTPUT },
    { "COM1:", "SER:",  DEVALIAS_BOTH   },
    { "COM2:", "SER:",  DEVALIAS_BOTH   }
};

static const AliasEntry mbasic_aliases[] = {
    { "LST:", "ERR:",  DEVALIAS_OUTPUT },
    { "PUN:", "FILE:", DEVALIAS_OUTPUT },
    { "RDR:", "FILE:", DEVALIAS_INPUT  }
};

#if 0
static const AliasEntry applesoft_aliases[] = {
    { "SLOT0:", "CON:",  DEVALIAS_BOTH   },
    { "SLOT1:", "ERR:",  DEVALIAS_OUTPUT },
    { "SLOT2:", "SER:",  DEVALIAS_BOTH   },
    { "SLOT6:", "FILE:", DEVALIAS_BOTH   }
};
#endif

static const AliasEntry ecma116_aliases[] = {
    { "PRINTER:", "ERR:", DEVALIAS_OUTPUT },
    { "TERM:",    "CON:", DEVALIAS_BOTH   }
};

#if 0
static const AliasEntry superbasic_aliases[] = {
    { "CON_",  "CON:",  DEVALIAS_BOTH   },
    { "SCR_",  "CON:",  DEVALIAS_OUTPUT },
    { "SER1",  "SER:",  DEVALIAS_BOTH   },
    { "SER2",  "SER:",  DEVALIAS_BOTH   },
    { "MDV1_", "FILE:", DEVALIAS_BOTH   },
    { "MDV2_", "FILE:", DEVALIAS_BOTH   },
    { "FLP1_", "FILE:", DEVALIAS_BOTH   },
    { "NET",   "NET:",  DEVALIAS_BOTH   }
};
#endif

static const AliasEntry sbasic_aliases[] = {
    { "TTY:", "CON:",  DEVALIAS_BOTH   },
    { "PTR:", "FILE:", DEVALIAS_INPUT  },
    { "PTP:", "FILE:", DEVALIAS_OUTPUT },
    { "LPT:", "ERR:",  DEVALIAS_OUTPUT }
};

#if 0
static const AliasEntry trs2_aliases[] = {
    { "*PR",  "ERR:",  DEVALIAS_OUTPUT },
    { "*CL",  "FILE:", DEVALIAS_INPUT  },
    { "*CS",  "FILE:", DEVALIAS_OUTPUT }
};
#endif

int device_alias_core_load_preset(DeviceAlias *table, int *count, int max_aliases, int dialect_id)
{
    const AliasEntry *entries = NULL;
    int ent_count = 0;
    int i, loaded = 0;

    switch (dialect_id) {
        case 0: // 0
            entries = gwbasic_aliases;
            ent_count = (int)(sizeof(gwbasic_aliases) / sizeof(gwbasic_aliases[0]));
            break;
        case 2: // 2
            entries = ecma116_aliases;
            ent_count = (int)(sizeof(ecma116_aliases) / sizeof(ecma116_aliases[0]));
            break;
        case 3: // 3
            entries = qbasic_aliases;
            ent_count = (int)(sizeof(qbasic_aliases) / sizeof(qbasic_aliases[0]));
            break;
        case 4: // 4
            entries = mbasic_aliases;
            ent_count = (int)(sizeof(mbasic_aliases) / sizeof(mbasic_aliases[0]));
            break;
        case 5: // 6
            entries = sbasic_aliases;
            ent_count = (int)(sizeof(sbasic_aliases) / sizeof(sbasic_aliases[0]));
            break;
        default:
            return 0;
    }

    for (i = 0; i < ent_count; i++) {
        if (device_alias_core_set(table, count, max_aliases, entries[i].alias, entries[i].target, entries[i].direction, dialect_id) == 0) {
            loaded++;
        }
    }
    return loaded;
}
