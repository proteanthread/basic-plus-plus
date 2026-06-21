/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: security.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Core interpreter engine infrastructure, memory pool allocator, error model, values, platform, security gating, and boot configurations.
 *
 * 2. WHAT TO EXPECT:
 *    Fixed memory footprint utilizing compile-time pool allocators (defined in config.h). Avoids malloc/free at runtime.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Diagnostic logs, specific error message phrasing, platform detection strings, security sandbox policy matrices.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    BValue tagged union structure fields, core memory allocator logic, security capability ratings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check config.h pool sizes (e.g. increase PROGRAM_MEMORY_SIZE). If security level is ratcheted, check security level enforcement policies.
 * ===================================================================== */

// ---
// BASIC++ Interpreter - security.c
// ---
//
// Security system implementation -- 6-level model with 19 operations.
//
// PURPOSE:
//   Provides a mandatory access control (MAC) system that restricts
//   which BASIC operations are permitted at each security level.
//   This protects against accidental or malicious misuse of
//   powerful features like SHELL, POKE, file deletion, and
//   network access.
//
// HOW IT WORKS:
//   The security system has three layers:
//
//   Layer 1 -- Security Levels (SEC_OPEN to SEC_PARANOID):
//     A static permission matrix maps each (level, operation) pair
//     to allowed (1) or denied (0). The matrix is hardcoded below
//     and cannot be modified at runtime (security principle).
//
//   Layer 2 -- RESTRICT Overrides:
//     Individual operations and keywords can be additionally
//     restricted via the SECURITY RESTRICT command. These are
//     additive -- they can deny access but never grant it.
//
//   Layer 3 -- Path and Port Checks:
//     File path security (no absolute paths, no .., extension
//     whitelist) and network port restrictions (well-known ports
//     plus ephemeral range).
//
//   Check flow:
//     1. security_check(op, line) is called.
//     2. If the operation is RESTRICT-overridden -> denied.
//     3. If current level is SEC_OPEN -> allowed (fast path).
//     4. Check the permission matrix -> allowed or denied.
//     5. If denied, print "SORRY? Security: ..." with details.
//
// ===================================================================
// SECURITY LEVEL REFERENCE
// ===================================================================
//
//   SEC_OPEN (0):
//     All operations permitted. No restrictions whatsoever.
//     Use for trusted environments or development.
//
//   SEC_SAFE (1):
//     Like OPEN but blocks: SYSTEM (shell), USB.
//     Good for running untrusted programs that need file I/O.
//
//   SEC_STANDARD (2):
//     Controlled sandbox. Blocks: file management (DELETE/RENAME),
//     compile, chain, system, USB, memory write (POKE).
//     Allows: file read/write, block I/O, modules, networking.
//     Default for most dialects.
//
//   SEC_EDUCATIONAL (3):
//     Classroom mode. Only allows: file read/write, stream I/O,
//     chain, modules, program management. Blocks everything else.
//     Safe for student use.
//
//   SEC_RESTRICTED (4):
//     Very limited. Only allows: file read, chain, modules,
//     program management. No writes, no networking, no PEEK/POKE.
//     For minimal dialects (PATB, TRS-80 L1).
//
//   SEC_PARANOID (5):
//     Pure computation only. Blocks ALL I/O, file access, memory
//     access, networking, modules, and everything except program
//     management (LIST/RUN). Only math and variables work.
//
// HOW TO EXTEND / CUSTOMIZE:
//   Adding a new security operation:
//   1. Add SECOP_YOUR_OP to the SecOperation enum in security.h.
//   2. Update SECOP_COUNT in security.h.
//   3. Add a column to the 'allowed' matrix below for each level.
//   4. Add the operation name to the 'op_names' array.
//   5. Call security_check(SECOP_YOUR_OP, line_num) where needed.
//
//   Adding a new security level:
//   1. Add SEC_YOUR_LEVEL to the SecLevel enum in security.h.
//   2. Update SEC_COUNT in security.h.
//   3. Add a row to the 'allowed' matrix below.
//   4. Add the level name to the 'level_names' array.
//
//   Adding custom error messages:
//   The error messages use printf with the operation name from
//   op_names[]. To customize, edit the strings in op_names[].
//
// FINE-TUNING:
//   - Default security level: Set in config.h via
//     BASICPP_DEFAULT_SECURITY, or per-dialect via
//     dialect_default_security() in dialect.c.
//
//   - Runtime adjustment: Use the BASIC command
//     SECURITY LEVEL "STANDARD" (or OPEN, SAFE, etc.)
//     to change at runtime. Level changes are immediate.
//
//   - RESTRICT overrides: Use SECURITY RESTRICT to block
//     specific operations beyond the level's defaults:
//       SECURITY RESTRICT "file write"
//       SECURITY RESTRICT "network sockets"
//     These persist until SECURITY RESET.
//
//   - Path security: The extension whitelist in
//     security_check_path() controls which file types can be
//     loaded/saved via external code loading. Edit the exts[]
//     array to add new allowed extensions.
//
//   - Port security: The well-known port list in
//     security_check_port() controls which network ports are
//     accessible. Add or remove ports as needed.
//
// TROUBLESHOOTING:
//   - "SORRY? Security: ... not permitted at level ...":
//     The operation is blocked by the current security level.
//     Check the permission matrix above. Solutions:
//       1. Lower the security level: SECURITY LEVEL "OPEN"
//       2. If this is a RESTRICT override, use SECURITY RESET
//
//   - "SORRY? Security: ... restricted via SECURITY RESTRICT":
//     The operation was individually restricted. Use:
//       SECURITY LIST    (see what's restricted)
//       SECURITY RESET   (clear all RESTRICT overrides)
//
//   - "SORRY? Security: absolute path ... not permitted":
//     Only relative paths are allowed for external code loading.
//     Use relative paths or mount the directory via VFS first.
//
//   - "SORRY? Security: file extension not allowed":
//     The file has an extension not in the whitelist. Only
//     .dll, .so, .lib, .bpl, .bpp, .bas, .spec, and .yaml
//     are permitted for external loading.
//
//   - "SORRY? Security: port ... not permitted":
//     The port is not in the allowed list. Only well-known
//     ports (21,22,23,25,53,80,etc.) and ephemeral ports
//     (1024-49151) are permitted.
//
// PERFORMANCE:
//   - security_check(): O(1) -- array lookup + fast path for OPEN.
//   - security_check_path(): O(n) for path traversal scan.
//   - security_check_port(): O(1) -- switch statement.
//   - RESTRICT checks: O(1) -- array indexed by operation.
//
// MINIMALIZATION:
//   This file is CORE tier -- required for any build.
//   For a truly minimal build (no security at all):
//   - Replace security_check() with a stub that always returns 0.
//   - Remove the permission matrix and level machinery.
//   - Saves ~3 KB code.
//
// DEPENDENCIES:
//   - security.h  (SecLevel, SecOperation, enums)
//   - module.h    (CAP_* capability flags for module check)
//   - platform.h  (platform_get_info for DOS memory check)
//
// C17 COMPLIANCE:
//   - Uses only C17 standard library (stdio.h, string.h, ctype.h).
//   - No platform-specific code (memory address check uses
//     platform_get_info() abstraction).
//   - Compiles cleanly on MSVC /std:c17 and gcc -std=c17.
//
// ---

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "security.h"
#include "module.h"
#include "platform.h"

// -----------------------------------------------------------------
// Module State
// -----------------------------------------------------------------

// Current security level (default: SEC_OPEN)
static SecLevel current_level = SEC_OPEN;

// SECURITY RESTRICT overrides -- per-operation deny flags
static int restrict_ops[SECOP_COUNT];
static int restrict_ops_count = 0;

// SECURITY RESTRICT keyword blacklist
#define MAX_RESTRICT_KEYWORDS 64
static int restrict_kw_ids[MAX_RESTRICT_KEYWORDS];
static int restrict_kw_count = 0;

// -----------------------------------------------------------------
// Permission Matrix
// -----------------------------------------------------------------
// allowed[level][operation] -- 1 = permitted, 0 = denied
//
// Operations (columns):
//   0=FILE_READ  1=FILE_WRITE 2=FILE_MGMT  3=FILE_BLOCK
//   4=FILE_STRM  5=COMPILE    6=CHAIN      7=SYSTEM
//   8=MODULE     9=USB       10=VDEV      11=VTERM
//  12=VCON      13=EVAL      14=NETWORK   15=MEM_READ
//  16=MEM_WRITE 17=PROG_MGMT 18=EXT_LOAD
//
// Levels (rows):
//   SEC_OPEN(0), SEC_SAFE(1), SEC_STANDARD(2),
//   SEC_EDUCATIONAL(3), SEC_RESTRICTED(4), SEC_PARANOID(5)
//
// How to customize:
//   To change what a level allows, edit the row below.
//   1 = allowed, 0 = denied. Do NOT reorder columns.
// -----------------------------------------------------------------
static const int allowed[SEC_COUNT][SECOP_COUNT] = {
    // SEC_OPEN (0): all operations permitted
    //  RD WR MG BK ST CO CH SY MO US VD VT VC EV NW MR MW PM EL
    {   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },

    // SEC_SAFE (1): secure but functional
    //  RD WR MG BK ST CO CH SY MO US VD VT VC EV NW MR MW PM EL
    {   1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 },

    // SEC_STANDARD (2): controlled sandbox
    //  RD WR MG BK ST CO CH SY MO US VD VT VC EV NW MR MW PM EL
    {   1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1 },

    // SEC_EDUCATIONAL (3): classroom mode
    //  RD WR MG BK ST CO CH SY MO US VD VT VC EV NW MR MW PM EL
    {   1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },

    // SEC_RESTRICTED (4): very limited
    //  RD WR MG BK ST CO CH SY MO US VD VT VC EV NW MR MW PM EL
    {   1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },

    // SEC_PARANOID (5): pure computation only
    //  RD WR MG BK ST CO CH SY MO US VD VT VC EV NW MR MW PM EL
    {   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 }
};

// -----------------------------------------------------------------
// Operation and Level Names (for error messages and SECURITY LIST)
// -----------------------------------------------------------------

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

// -----------------------------------------------------------------
// Core Security API
// -----------------------------------------------------------------

// security_init - Initialize the security system.
//
// Sets the security level and clears all RESTRICT overrides.
// Called once during boot (from boot.c).
//
// Parameters:
//   level - initial security level (SEC_OPEN to SEC_PARANOID)
//
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

// security_get_level - Return the current security level.
SecLevel security_get_level(void)
{
    return current_level;
}

// security_set_level - Change the security level at runtime.
//
// BASIC usage: SECURITY LEVEL "STANDARD"
//
// Parameters:
//   level - new security level (must be valid)
//
void security_set_level(SecLevel level)
{
    if (level >= 0 && level < SEC_COUNT) {
        current_level = level;
    }
}

// security_level_name - Return the human-readable name for a level.
//
// Returns "OPEN", "SAFE", "STANDARD", "EDUCATIONAL",
// "RESTRICTED", "PARANOID", or "UNKNOWN".
//
const char *security_level_name(SecLevel level)
{
    if (level >= 0 && level < SEC_COUNT) {
        return level_names[level];
    }
    return "UNKNOWN";
}

// security_find_level_by_name - Look up a security level by name.
//
// Case-insensitive exact match against level names.
//
// Parameters:
//   name - level name to search for (e.g., "STANDARD")
//
// Returns:
//   Level index (0-5) if found, -1 if not found.
//
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

// -----------------------------------------------------------------
// Security Check -- Main Gate Function
// -----------------------------------------------------------------

// security_check - Check if an operation is permitted.
//
// This is the main security gate function. Called before any
// security-sensitive operation (file I/O, SHELL, POKE, etc.).
//
// Check order:
//   1. RESTRICT override (always denies if set)
//   2. SEC_OPEN fast path (always permits)
//   3. Permission matrix lookup
//
// Parameters:
//   op       - the operation to check (SECOP_FILE_READ, etc.)
//   line_num - BASIC line number (for error message context)
//
// Returns:
//   0 if permitted
//  -1 if denied (prints error message)
//
int security_check(SecOperation op, int line_num)
{
    if (op < 0 || op >= SECOP_COUNT) return -1;

    // Check RESTRICT overrides first (always deny if set)
    if (restrict_ops[op]) {
        printf("SORRY? Security: %s restricted "
            "via SECURITY RESTRICT",
            op_names[op]);
        if (line_num > 0) printf(" in line %d", line_num);
        printf("\n");
        return -1;
    }

    // Fast path: SEC_OPEN permits everything
    if (current_level == SEC_OPEN) return 0;

    // Check the permission matrix
    if (allowed[current_level][op]) {
        return 0;  // permitted
    }

    // Denied -- print error with context
    printf("SORRY? Security: %s not permitted "
        "at level %s",
        op_names[op], level_names[current_level]);
    if (line_num > 0) {
        printf(" in line %d", line_num);
    }
    printf("\n");
    return -1;
}

// -----------------------------------------------------------------
// Module Capability Check
// -----------------------------------------------------------------

// security_module_allowed - Check if a module's capabilities are
// permitted at the current security level.
//
// Modules declare their capabilities via a bitmask (CAP_IO,
// CAP_FILE, CAP_NETWORK, etc.). This function checks whether
// those capabilities are compatible with the current level.
//
// Parameters:
//   capabilities - bitmask of CAP_* flags from the module
//
// Returns:
//   1 if permitted
//   0 if denied
//
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

// -----------------------------------------------------------------
// Pinned Level Check (for plugins)
// -----------------------------------------------------------------

// security_check_pinned_level - Check if a plugin's required level
// is compatible with the current level.
//
// Plugins can declare a minimum required security level in their
// manifest (plugin.yaml). This check ensures the current level
// meets or exceeds that requirement.
//
// Parameters:
//   required_level - the plugin's required level
//
// Returns:
//   1 if compatible (current >= required or no requirement)
//   0 if incompatible
//
int security_check_pinned_level(SecLevel required_level)
{
    if (required_level == SEC_COUNT) return 1;  // no requirement
    if (current_level == SEC_PARANOID) return 0;
    return ((int)current_level >= (int)required_level) ? 1 : 0;
}

// -----------------------------------------------------------------
// Memory Access Check
// -----------------------------------------------------------------

// security_check_mem - Check if PEEK/POKE to an address is permitted.
//
// On DOS: all addresses are accessible at SAFE/STANDARD.
// On modern OSes: only addresses below 0x10000 are accessible
// (emulated low memory for legacy BASIC compatibility).
//
// Parameters:
//   address - memory address to access
//   size    - number of bytes (reserved, currently unused)
//
// Returns:
//   0 if permitted
//  -1 if denied
//
int security_check_mem(unsigned long address, int size)
{
    const PlatformInfo *plat;

    (void)size;  // reserved for future granular checks

    if (current_level == SEC_OPEN) return 0;

    plat = platform_get_info();
    if (plat->id == PLAT_DOS) {
        // DOS: all conventional memory is accessible at SAFE/STANDARD
        if (current_level == SEC_SAFE ||
            current_level == SEC_STANDARD) return 0;
    } else {
        // Modern OS: only emulated low memory (< 64K)
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

// -----------------------------------------------------------------
// Network Port Check
// -----------------------------------------------------------------

// security_check_port - Check if a network port is accessible.
//
// At SEC_SAFE and SEC_STANDARD: well-known ports (21, 22, 80, 443,
// etc.) and ephemeral ports (1024-49151) are allowed.
// At SEC_EDUCATIONAL and above: no network access.
//
// Parameters:
//   port     - TCP/UDP port number
//   line_num - BASIC line number for error context
//
// Returns:
//   0 if permitted
//  -1 if denied
//
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

    // SEC_SAFE / SEC_STANDARD: allow ephemeral ports
    if (port >= 1024 && port <= 49151) return 0;

    // Well-known service ports whitelist
    switch (port) {
    case 21:    case 22:    case 23:    case 25:
    case 53:    case 80:    case 110:   case 119:
    case 143:   case 161:   case 162:   case 443:
    case 465:   case 587:   case 993:   case 995:
    case 6667:  case 6697:  case 8080:  case 8443:
    case 16384:
        return 0;  // allowed
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

// -----------------------------------------------------------------
// File Path Security
// -----------------------------------------------------------------

// security_check_path - Validate a file path for external loading.
//
// Used by the plugin/module loader to ensure loaded files are safe.
// Checks:
//   1. Path must not be empty.
//   2. No absolute paths (/ or \ or X:\).
//   3. No path traversal (..).
//   4. File extension must be in the whitelist.
//
// Extension whitelist: .dll, .so, .lib, .bpl, .bpp, .bas, .spec, .yaml
//
// How to customize:
//   To add a new allowed extension, add it to the exts[] array below.
//
// Parameters:
//   path     - file path to validate
//   line_num - BASIC line number for error context
//
// Returns:
//   0 if the path is safe
//  -1 if rejected (prints error message)
//
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

    // Reject absolute paths (Unix: /path, Windows: \path or C:\path)
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

    // Reject path traversal attempts (..)
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

    // Check file extension against whitelist
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

// -----------------------------------------------------------------
// SECURITY RESTRICT API
// -----------------------------------------------------------------
// Allows runtime restriction of individual operations and keywords
// beyond what the security level enforces. These overrides are
// additive (can only deny, never grant) and persist until
// SECURITY RESET.
//
// BASIC usage:
//   SECURITY RESTRICT "file write"     ' block file writes
//   SECURITY RESTRICT KEYWORD "SHELL"  ' block SHELL command
//   SECURITY LIST                      ' show restrictions
//   SECURITY RESET                     ' clear all restrictions
// -----------------------------------------------------------------

// security_restrict_op - Block a specific operation.
//
// Parameters:
//   op - SecOperation to block
//
// Returns:
//   0 on success, -1 if invalid operation
//
int security_restrict_op(SecOperation op)
{
    if (op < 0 || op >= SECOP_COUNT) return -1;
    if (!restrict_ops[op]) {
        restrict_ops[op] = 1;
        restrict_ops_count++;
    }
    return 0;
}

// security_is_op_restricted - Check if an operation is RESTRICT-blocked.
//
// Returns:
//   1 if the operation is restricted, 0 otherwise
//
int security_is_op_restricted(SecOperation op)
{
    if (op < 0 || op >= SECOP_COUNT) return 0;
    return restrict_ops[op];
}

// security_restrict_keyword - Block a specific BASIC keyword.
//
// Parameters:
//   kw_id - keyword ID to block (from lexer.h)
//
// Returns:
//   0 on success, -1 if the keyword list is full
//
int security_restrict_keyword(int kw_id)
{
    int i;
    // Check if already restricted
    for (i = 0; i < restrict_kw_count; i++) {
        if (restrict_kw_ids[i] == kw_id) return 0;
    }
    if (restrict_kw_count >= MAX_RESTRICT_KEYWORDS) return -1;
    restrict_kw_ids[restrict_kw_count++] = kw_id;
    return 0;
}

// security_is_keyword_restricted - Check if a keyword is RESTRICT-blocked.
//
// Returns:
//   1 if the keyword is restricted, 0 otherwise
//
int security_is_keyword_restricted(int kw_id)
{
    int i;
    for (i = 0; i < restrict_kw_count; i++) {
        if (restrict_kw_ids[i] == kw_id) return 1;
    }
    return 0;
}

// security_restrict_list - Display all active RESTRICT overrides.
//
// Called by the SECURITY LIST command. Shows both operation-level
// and keyword-level restrictions.
//
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

// security_restrict_count - Return total number of active restrictions.
int security_restrict_count(void)
{
    return restrict_ops_count + restrict_kw_count;
}
