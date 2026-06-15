/*
 * ---
 * BASIC++ Interpreter - device_alias.h
 * ---
 *
 * Dialect Device Alias System.
 *
 * PURPOSE:
 * Maps legacy device names from classic BASIC dialects to the
 * modern VDev system. When an Atari program says OPEN "E:", the
 * alias system translates "E:" to "CON:" transparently.
 *
 * DESIGN:
 * A static table of up to MAX_DEVICE_ALIASES entries. Each entry
 * maps a source name (the dialect device) to a target VDev name,
 * with a direction flag (input, output, or bidirectional).
 *
 * Aliases are dialect-specific. When a dialect is initialized,
 * device_alias_load_dialect() populates the alias table with the
 * correct mappings for that system. Aliases can also be created
 * manually via the DEVICE ALIAS command.
 *
 * Aliases are opt-in by default. Each dialect's apply() callback
 * decides whether to auto-load aliases. Dialects with strong
 * device identity (Atari, C64, CoCo) opt-in automatically.
 * Dialects where device names already match modern conventions
 * (GW-BASIC, QBasic) are opt-out.
 *
 * ALIAS RESOLUTION:
 * When fileio_open() receives a filename, it calls
 * device_alias_resolve() BEFORE attempting a filesystem open.
 * If the name matches an alias, the I/O is routed through the
 * mapped VDev instead of the filesystem.
 *
 * ---
 */

#ifndef BASICPP_DEVICE_ALIAS_H
#define BASICPP_DEVICE_ALIAS_H

#include "dialect.h"

/* Maximum number of device aliases */
#define MAX_DEVICE_ALIASES 32

/* Direction flags — what I/O operations the alias supports */
#define DEVALIAS_INPUT   0x01  /* alias supports reading */
#define DEVALIAS_OUTPUT  0x02  /* alias supports writing */
#define DEVALIAS_BOTH    0x03  /* alias supports read + write */

/*
 * DeviceAlias - Single alias mapping entry.
 *
 * alias:     Source device name (e.g., "E:", "S:", "K:")
 * target:    Target VDev name (e.g., "CON:", "ERR:", "FILE:")
 * direction: DEVALIAS_INPUT, DEVALIAS_OUTPUT, or DEVALIAS_BOTH
 * dialect:   Which dialect registered this alias (for tracking)
 * active:    1 if alias is live, 0 if disabled
 */
typedef struct DeviceAlias {
    char alias[16];       /* source device name (upper-cased) */
    char target[16];      /* target VDev name */
    int direction;        /* DEVALIAS_* flags */
    DialectId dialect;    /* owning dialect (or -1 for manual) */
    int active;           /* 1 = active, 0 = disabled */
} DeviceAlias;

/*
 * device_alias_init - Clear the alias table.
 *
 * Must be called once during boot, before any dialect
 * initialization. Zeros all entries and sets count to 0.
 */
void device_alias_init(void);

/*
 * device_alias_register - Add a single alias mapping.
 *
 * Parameters:
 *   alias     - source device name (e.g., "E:")
 *   target    - target VDev name (e.g., "CON:")
 *   direction - DEVALIAS_INPUT, DEVALIAS_OUTPUT, or DEVALIAS_BOTH
 *   dialect   - owning dialect ID, or -1 for manual entries
 *
 * Returns 0 on success, -1 if the table is full.
 * If the alias already exists, it is updated (not duplicated).
 */
int device_alias_register(const char *alias, const char *target,
                          int direction, int dialect);

/*
 * device_alias_resolve - Look up a device alias.
 *
 * Given a filename/device name, check if it matches any active
 * alias. If so, return a pointer to the DeviceAlias entry.
 * If not, return NULL (caller should treat as a regular file).
 *
 * Matching is case-insensitive.
 *
 * Parameters:
 *   name - the device/file name to resolve
 *
 * Returns: pointer to DeviceAlias, or NULL if no match.
 */
const DeviceAlias *device_alias_resolve(const char *name);

/*
 * device_alias_load_dialect - Load all aliases for a dialect.
 *
 * Clears any existing aliases from the same dialect, then
 * populates the table with the standard device mappings for
 * that platform.
 *
 * Parameters:
 *   id - the dialect to load aliases for
 *
 * Returns the number of aliases loaded.
 */
int device_alias_load_dialect(DialectId id);

/*
 * device_alias_clear_dialect - Remove all aliases for a dialect.
 *
 * Parameters:
 *   id - the dialect whose aliases to remove
 */
void device_alias_clear_dialect(DialectId id);

/*
 * device_alias_clear_all - Remove all aliases (reset).
 */
void device_alias_clear_all(void);

/*
 * device_alias_list - Print all active aliases.
 *
 * Shows alias name, target VDev, direction, and owning dialect.
 */
void device_alias_list(void);

/*
 * device_alias_count - Return number of active aliases.
 */
int device_alias_count(void);

/*
 * device_alias_remove - Remove a specific alias by name.
 *
 * Returns 0 on success, -1 if not found.
 */
int device_alias_remove(const char *alias);

/*
 * device_alias_set_active - Enable or disable an alias.
 *
 * Parameters:
 *   alias  - the alias name to toggle
 *   active - 1 to enable, 0 to disable
 *
 * Returns 0 on success, -1 if not found.
 */
int device_alias_set_active(const char *alias, int active);

#endif /* BASICPP_DEVICE_ALIAS_H */
