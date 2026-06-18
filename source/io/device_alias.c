 // ---
 // BASIC++ Interpreter - device_alias.c
 // ---
 //
 // Dialect Device Alias System -- Implementation.
 //
 // Maps legacy device names from classic BASIC platforms to
 // the modern VDev system:
 //
 //   Atari E:  ->  CON:    (screen editor)
 //   Atari K:  ->  CON:    (keyboard input)
 //   Atari P:  ->  ERR:    (printer -> stderr fallback)
 //   C64  #0   ->  CON:    (keyboard/screen)
 //   CoCo #-2  ->  ERR:    (printer)
 //   GW   SCRN:->  CON:    (screen output)
 //   etc.
 //
 // The alias table is a fixed-size static array. Aliases are
 // loaded per-dialect via device_alias_load_dialect() and can
 // also be created manually via the DEVICE ALIAS command.
 //
//
// HOW TO EXTEND:
//   See the preamble comments in related files for
//   customization and extension instructions.
//
// TROUBLESHOOTING:
//   Check error_occurred() after operations that can fail.
//   Use error_raise(ERR_xxx, line_num) for error reporting.
 // ---

#include <stdio.h>
#include <string.h>
#include "device_alias.h"

// --- Alias Table ---
 // Static table of MAX_DEVICE_ALIASES entries.
 // Active entries have active == 1.
static DeviceAlias alias_table[MAX_DEVICE_ALIASES];
static int alias_count = 0;

// --- Internal: case-insensitive string compare ---
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

// --- Internal: copy string, upper-case, bounded ---
static void da_copy_upper(char *dst, const char *src, int max)
{
    int i;
    for (i = 0; i < max - 1 && src[i] != '\0'; i++) {
        dst[i] = src[i];
        if (dst[i] >= 'a' && dst[i] <= 'z')
            dst[i] = (char)(dst[i] - 32);
    }
    dst[i] = '\0';
}

// --- Internal: find existing alias by name ---
static int da_find(const char *alias)
{
    int i;
    for (i = 0; i < alias_count; i++) {
        if (da_str_eq_ci(alias_table[i].alias, alias))
            return i;
    }
    return -1;
}

// --- device_alias_init ---
void device_alias_init(void)
{
    memset(alias_table, 0, sizeof(alias_table));
    alias_count = 0;
}

// --- device_alias_register ---
int device_alias_register(const char *alias, const char *target,
                          int direction, int dialect)
{
    int idx;

    if (alias == NULL || target == NULL) return -1;

    // Check for existing -- update in place
    idx = da_find(alias);
    if (idx >= 0) {
        da_copy_upper(alias_table[idx].target, target, 16);
        alias_table[idx].direction = direction;
        alias_table[idx].dialect = (DialectId)dialect;
        alias_table[idx].active = 1;
        return 0;
    }

    // New entry
    if (alias_count >= MAX_DEVICE_ALIASES) return -1;

    idx = alias_count;
    da_copy_upper(alias_table[idx].alias, alias, 16);
    da_copy_upper(alias_table[idx].target, target, 16);
    alias_table[idx].direction = direction;
    alias_table[idx].dialect = (DialectId)dialect;
    alias_table[idx].active = 1;
    alias_count++;
    return 0;
}

// --- device_alias_resolve ---
const DeviceAlias *device_alias_resolve(const char *name)
{
    int i;
    if (name == NULL) return NULL;

    for (i = 0; i < alias_count; i++) {
        if (alias_table[i].active &&
            da_str_eq_ci(alias_table[i].alias, name)) {
            return &alias_table[i];
        }
    }
    return NULL;
}

// --- device_alias_clear_dialect ---
void device_alias_clear_dialect(DialectId id)
{
    int i;
    for (i = 0; i < alias_count; i++) {
        if (alias_table[i].dialect == id) {
            alias_table[i].active = 0;
        }
    }
}

// --- device_alias_clear_all ---
void device_alias_clear_all(void)
{
    memset(alias_table, 0, sizeof(alias_table));
    alias_count = 0;
}

// --- device_alias_remove ---
int device_alias_remove(const char *alias)
{
    int idx = da_find(alias);
    if (idx < 0) return -1;
    alias_table[idx].active = 0;
    return 0;
}

// --- device_alias_set_active ---
int device_alias_set_active(const char *alias, int active)
{
    int idx = da_find(alias);
    if (idx < 0) return -1;
    alias_table[idx].active = active ? 1 : 0;
    return 0;
}

// --- device_alias_count ---
int device_alias_count(void)
{
    int i, n = 0;
    for (i = 0; i < alias_count; i++) {
        if (alias_table[i].active) n++;
    }
    return n;
}

// --- device_alias_list ---
void device_alias_list(void)
{
    int i, found = 0;

    printf("%-10s %-10s %-8s %s\n",
           "Alias", "Target", "Dir", "Dialect");
    printf("%-10s %-10s %-8s %s\n",
           "-----", "------", "---", "-------");

    for (i = 0; i < alias_count; i++) {
        const char *dir_str;
        const DialectConfig *dc;

        if (!alias_table[i].active) continue;
        found++;

        switch (alias_table[i].direction) {
        case DEVALIAS_INPUT:  dir_str = "IN";    break;
        case DEVALIAS_OUTPUT: dir_str = "OUT";   break;
        case DEVALIAS_BOTH:   dir_str = "BOTH";  break;
        default:              dir_str = "?";     break;
        }

        if (alias_table[i].dialect >= 0 &&
            alias_table[i].dialect < DIALECT_COUNT) {
            dc = dialect_get_config();
            // We just show the dialect short name if it
             // matches the owning dialect, else "(manual)" 
            if (alias_table[i].dialect == dc->id) {
                printf("%-10s %-10s %-8s %s\n",
                       alias_table[i].alias,
                       alias_table[i].target,
                       dir_str,
                       dc->short_name);
            } else {
                printf("%-10s %-10s %-8s (dialect %d)\n",
                       alias_table[i].alias,
                       alias_table[i].target,
                       dir_str,
                       alias_table[i].dialect);
            }
        } else {
            printf("%-10s %-10s %-8s (manual)\n",
                   alias_table[i].alias,
                   alias_table[i].target,
                   dir_str);
        }
    }

    if (!found) {
        printf("(no active aliases)\n");
    }
}


// ===================================================================
 // DIALECT-SPECIFIC ALIAS TABLES
 // ===================================================================
 //
 // Each dialect defines a static table of device aliases that map
 // its native device names to modern VDev equivalents.
 //
 // MAPPING PHILOSOPHY:
 // - E: (editor/screen) -> CON: (stdout+stdin)
 // - S: (screen display) -> CON: (stdout only)
 // - K: (keyboard) -> CON: (stdin only)
 // - P: (printer) -> ERR: (stderr as fallback)
 // - D: / D1: (disk) -> FILE: (filesystem)
 // - C: (cassette) -> FILE: (tape emulation)
 // - R: (RS-232) -> SER: (when registered)
 // - LPT1: -> ERR: or LPT: (when registered)
 // - COM1: -> SER: (when registered)
 // - SCRN: / KYBD: -> CON: (screen/keyboard)
 //
 // If a target VDev (like SER: or LPT:) doesn't exist, the
 // alias still resolves but the VDev lookup will fail gracefully
 // with an error message.

// Helper: bulk-register from a static array
typedef struct {
    const char *alias;
    const char *target;
    int direction;
} AliasEntry;

static int da_load_table(const AliasEntry *entries, int count,
                         DialectId id)
{
    int i, loaded = 0;
    for (i = 0; i < count; i++) {
        if (device_alias_register(entries[i].alias,
                                  entries[i].target,
                                  entries[i].direction,
                                  (int)id) == 0) {
            loaded++;
        }
    }
    return loaded;
}


// --- Atari BASIC (Shepardson, 1979) ---
 //
 // CIO device names:
 //   E: = Screen Editor (bidirectional)
 //   S: = Screen Display (output only -- no cursor editing)
 //   K: = Keyboard (input only)
 //   P: = Printer (output only)
 //   D: = Disk Drive 1 (D1: equivalent)
 //   D1: = Disk Drive 1
 //   D2: = Disk Drive 2
 //   C: = Cassette
 //   R: = RS-232 serial
 //   R1: = RS-232 port 1
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

// --- Commodore 64 BASIC v2 (1982) ---
 //
 // Kernal device numbers:
 //   0 = Keyboard (input)
 //   1 = Cassette (datasette)
 //   2 = RS-232 (user port)
 //   3 = Screen (output)
 //   4 = Printer (serial bus)
 //   5 = Printer (secondary)
 //   8-15 = Disk drives (serial bus)
 //
 // C64 uses numeric device IDs with OPEN:
 //   OPEN 1,8,15,"S0:FILE"
 // We map the device portion to VDevs.
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

// --- Tandy CoCo BASIC (1980) ---
 //
 // CoCo uses channel numbers with special meaning:
 //   #-1 = Cassette
 //   #-2 = Printer
 //   #0 = Screen (bidirectional)
 //   Positive channels = disk files
 //
 // Since negative channels are unusual, we map them to
 // named aliases that the parser can detect.
static const AliasEntry coco_aliases[] = {
    { "CAS:",  "FILE:", DEVALIAS_BOTH   },
    { "LPT:",  "ERR:",  DEVALIAS_OUTPUT },
    { "SCR:",  "CON:",  DEVALIAS_BOTH   }
};

// --- Sinclair BASIC (ZX Spectrum, 1982) ---
 //
 // Sinclair streams:
 //   #0 = Lower screen (status area)
 //   #1 = Upper screen (main display)
 //   #2 = Printer (ZX Printer)
 //   #3 = Network (Interface 1 RS-232)
static const AliasEntry sinclair_aliases[] = {
    { "LOWER:", "CON:",  DEVALIAS_OUTPUT },
    { "UPPER:", "CON:",  DEVALIAS_BOTH   },
    { "ZXPRN:", "ERR:",  DEVALIAS_OUTPUT },
    { "ZXNET:", "NET:",  DEVALIAS_BOTH   }
};

// --- GW-BASIC / BASICA (IBM PC, 1983) ---
 //
 // GW-BASIC device names:
 //   SCRN: = Screen output
 //   KYBD: = Keyboard input
 //   LPT1: = Parallel printer
 //   COM1: = Serial port 1
 //   COM2: = Serial port 2
 //   CAS1: = Cassette (PCjr)
 //
 // These are already close to the modern convention
 // (CON: matches most), but we map for completeness.
static const AliasEntry gwbasic_aliases[] = {
    { "SCRN:", "CON:",  DEVALIAS_OUTPUT },
    { "KYBD:", "CON:",  DEVALIAS_INPUT  },
    { "LPT1:", "ERR:",  DEVALIAS_OUTPUT },
    { "LPT2:", "ERR:",  DEVALIAS_OUTPUT },
    { "COM1:", "SER:",  DEVALIAS_BOTH   },
    { "COM2:", "SER:",  DEVALIAS_BOTH   },
    { "CAS1:", "FILE:", DEVALIAS_BOTH   }
};

// --- QBasic (Microsoft, 1991) ---
 //
 // QBasic device names:
 //   CONS: = Console (bidirectional)
 //   SCRN: = Screen output
 //   LPT1: = Printer
 //   COM1: = Serial port
static const AliasEntry qbasic_aliases[] = {
    { "CONS:", "CON:",  DEVALIAS_BOTH   },
    { "SCRN:", "CON:",  DEVALIAS_OUTPUT },
    { "LPT1:", "ERR:",  DEVALIAS_OUTPUT },
    { "COM1:", "SER:",  DEVALIAS_BOTH   },
    { "COM2:", "SER:",  DEVALIAS_BOTH   }
};

// --- MBASIC / CP/M (Microsoft, 1977) ---
 //
 // CP/M standard device names:
 //   CON: = Console (already matches)
 //   LST: = List device (printer)
 //   PUN: = Paper tape punch
 //   RDR: = Paper tape reader
static const AliasEntry mbasic_aliases[] = {
    { "LST:", "ERR:",  DEVALIAS_OUTPUT },
    { "PUN:", "FILE:", DEVALIAS_OUTPUT },
    { "RDR:", "FILE:", DEVALIAS_INPUT  }
};

// --- AppleSoft BASIC (Apple II, 1977) ---
 //
 // Apple II uses slot-based I/O:
 //   PR#n = Output to slot n (PR#0 = screen)
 //   IN#n = Input from slot n (IN#0 = keyboard)
 //   Slot 1 = Printer
 //   Slot 2 = Serial/Modem
 //   Slot 6 = Disk II controller
 //
 // These are command-based, not device-named, but we provide
 // aliases for the logical devices.
static const AliasEntry applesoft_aliases[] = {
    { "SLOT0:", "CON:",  DEVALIAS_BOTH   },
    { "SLOT1:", "ERR:",  DEVALIAS_OUTPUT },
    { "SLOT2:", "SER:",  DEVALIAS_BOTH   },
    { "SLOT6:", "FILE:", DEVALIAS_BOTH   }
};

// --- Apple Integer BASIC (Wozniak, 1976) ---
 // Same slot model as AppleSoft.

// --- ECMA-116 Full BASIC (1986) ---
 //
 // ECMA-116 uses generic channel numbers.
 // Device #0 = terminal (standard).
static const AliasEntry ecma116_aliases[] = {
    { "TERM:", "CON:", DEVALIAS_BOTH }
};

// --- SuperBASIC (Sinclair QL, 1984) ---
 //
 // QL device names:
 //   CON_ = Console window
 //   SCR_ = Screen window (output only)
 //   SER1 = Serial port 1
 //   SER2 = Serial port 2
 //   MDV1_ = Microdrive 1
 //   MDV2_ = Microdrive 2
 //   FLP1_ = Floppy drive 1
 //   NET = Network
 //
 // Note: QL uses underscore, not colon, for some devices.
 // We normalize to colon convention.
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

// --- Tymshare SUPER BASIC (SDS-940, 1968) ---
 //
 // Tymshare used TTY-based I/O:
 //   TTY = Terminal (teletype)
 //   PTR = Paper tape reader
 //   PTP = Paper tape punch
 //   LPT = Line printer
static const AliasEntry sbasic_aliases[] = {
    { "TTY:", "CON:",  DEVALIAS_BOTH   },
    { "PTR:", "FILE:", DEVALIAS_INPUT  },
    { "PTP:", "FILE:", DEVALIAS_OUTPUT },
    { "LPT:", "ERR:",  DEVALIAS_OUTPUT }
};

// --- TRS-80 Level II (Microsoft, 1978) ---
 //
 // TRS-80 uses simple device letters:
 //   *PR = Printer
 //   *CL = Cassette (load)
 //   *CS = Cassette (save)
 //   Disk files use normal names through TRSDOS
static const AliasEntry trs2_aliases[] = {
    { "*PR",  "ERR:",  DEVALIAS_OUTPUT },
    { "*CL",  "FILE:", DEVALIAS_INPUT  },
    { "*CS",  "FILE:", DEVALIAS_OUTPUT }
};

// ===================================================================
 // device_alias_load_dialect - Master loader
 // =================================================================== 
int device_alias_load_dialect(DialectId id)
{
    // Clear previous aliases from this dialect
    device_alias_clear_dialect(id);

    switch (id) {
    case DIALECT_ATARI_MS:
        return da_load_table(atari_aliases,
            (int)(sizeof(atari_aliases) /
                  sizeof(atari_aliases[0])), id);

    case DIALECT_COMMODORE:
        return da_load_table(c64_aliases,
            (int)(sizeof(c64_aliases) /
                  sizeof(c64_aliases[0])), id);

    case DIALECT_COCO:
        return da_load_table(coco_aliases,
            (int)(sizeof(coco_aliases) /
                  sizeof(coco_aliases[0])), id);

    case DIALECT_SINCLAIR:
        return da_load_table(sinclair_aliases,
            (int)(sizeof(sinclair_aliases) /
                  sizeof(sinclair_aliases[0])), id);

    case DIALECT_GW_BASIC:
        return da_load_table(gwbasic_aliases,
            (int)(sizeof(gwbasic_aliases) /
                  sizeof(gwbasic_aliases[0])), id);

    case DIALECT_QBASIC:
        return da_load_table(qbasic_aliases,
            (int)(sizeof(qbasic_aliases) /
                  sizeof(qbasic_aliases[0])), id);

    case DIALECT_MBASIC:
        return da_load_table(mbasic_aliases,
            (int)(sizeof(mbasic_aliases) /
                  sizeof(mbasic_aliases[0])), id);

    case DIALECT_APPLESOFT:
        return da_load_table(applesoft_aliases,
            (int)(sizeof(applesoft_aliases) /
                  sizeof(applesoft_aliases[0])), id);

    case DIALECT_APPLE_INT:
        return da_load_table(applesoft_aliases,
            (int)(sizeof(applesoft_aliases) /
                  sizeof(applesoft_aliases[0])), id);

    case DIALECT_ECMA116:
        return da_load_table(ecma116_aliases,
            (int)(sizeof(ecma116_aliases) /
                  sizeof(ecma116_aliases[0])), id);

    case DIALECT_SUPERBASIC:
        return da_load_table(superbasic_aliases,
            (int)(sizeof(superbasic_aliases) /
                  sizeof(superbasic_aliases[0])), id);

    case DIALECT_SBASIC:
        return da_load_table(sbasic_aliases,
            (int)(sizeof(sbasic_aliases) /
                  sizeof(sbasic_aliases[0])), id);

    case DIALECT_TRS80_L2:
        return da_load_table(trs2_aliases,
            (int)(sizeof(trs2_aliases) /
                  sizeof(trs2_aliases[0])), id);

    // Dialects with no special device names
    case DIALECT_TINY_BASIC:
    case DIALECT_TRS80_L1:
    case DIALECT_ECMA55:
    default:
        return 0;
    }
}
