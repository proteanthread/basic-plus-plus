/*
 * ---
 * BASIC++ Interpreter - security.h
 * ---
 *
 * Security system interface.
 *
 * PURPOSE:
 * Enforces capability-gated access control on sensitive operations.
 * Every file I/O, compilation, chain, and system-level operation
 * passes through security_check() before executing.
 *
 * SECURITY LEVELS:
 * SEC_OPEN - No restrictions (default, matches pre)
 * SEC_STANDARD - File I/O allowed, COMPILE/CHAIN/SYSTEM blocked
 * SEC_RESTRICTED - Math/string only, all I/O blocked
 *
 * OPERATIONS:
 * Each sensitive operation has a SecOperation code. The permission
 * matrix maps (SecLevel x SecOperation) -> allowed/denied.
 *
 * ---
 */

#ifndef BASICPP_SECURITY_H
#define BASICPP_SECURITY_H

/* --- Security Levels ---
 */
typedef enum SecLevel {
 SEC_OPEN = 0, /* no restrictions */
 SEC_STANDARD = 1, /* file I/O yes, system ops no */
 SEC_RESTRICTED = 2, /* math/string only */
 SEC_COUNT = 3 /* sentinel */
} SecLevel;

/* --- Securable Operations ---
 */
typedef enum SecOperation {
    SECOP_FILE_READ = 0,    /* LOAD, BLOAD, MERGE, INPUT# */
    SECOP_FILE_WRITE = 1,   /* SAVE, BSAVE, OPEN, PRINT# */
    SECOP_FILE_MGMT = 2,    /* MKDIR, RMDIR, CHDIR, KILL, NAME */
    SECOP_FILE_BLOCK = 3,   /* GET, PUT records */
    SECOP_FILE_STREAM = 4,  /* Sequential stream I/O */
    SECOP_COMPILE = 5,      /* COMPILE command */
    SECOP_CHAIN = 6,        /* CHAIN command */
    SECOP_SYSTEM = 7,       /* system-level operations */
    SECOP_MODULE = 8,       /* module activation */
    SECOP_USB = 9,          /* USB hardware module access */
    SECOP_VDEV = 10,        /* Virtual device manipulation */
    SECOP_VTERM = 11,       /* Terminal interceptors */
    SECOP_VCON = 12,        /* Console output hijack */
    SECOP_EVAL = 13,        /* Dynamic string execution (exec_line) */
    SECOP_NETWORK = 14,     /* TCP/UDP Raw Sockets */
    SECOP_MEM_READ = 15,    /* PEEK memory read */
    SECOP_MEM_WRITE = 16,   /* POKE memory write */
    SECOP_COUNT = 17        /* sentinel */
} SecOperation;

/* --- Security API ---
 */

/*
 * security_init - Initialize the security system.
 *
 * Sets the active security level. Call once at boot.
 * Default should be SEC_OPEN for backward compatibility.
 */
void security_init(SecLevel level);

/*
 * security_get_level - Get the current security level.
 */
SecLevel security_get_level(void);

/*
 * security_set_level - Set the security level.
 *
 * Takes effect immediately for all subsequent operations.
 */
void security_set_level(SecLevel level);

/*
 * security_level_name - Get human-readable level name.
 *
 * Returns "OPEN", "STANDARD", or "RESTRICTED".
 */
const char *security_level_name(SecLevel level);

/*
 * security_check - Check if an operation is permitted.
 *
 * Returns 0 if allowed. Returns -1 and raises ERR_SORRY
 * with a descriptive message if denied.
 *
 * Parameters:
 * op - the operation to check
 * line_num - BASIC line number (for error messages)
 */
int security_check(SecOperation op, int line_num);

/*
 * security_module_allowed - Check if a module's capabilities
 * are permitted under the current security level.
 *
 * Returns 1 if allowed, 0 if denied.
 */
int security_module_allowed(unsigned int capabilities);

/*
 * security_check_mem - Validate memory access bounds.
 *
 * Checks if the specified address and size fall within the
 * simulated BASIC memory sandbox. Denies native pointer access
 * under RESTRICTED or STANDARD modes on modern OS architectures.
 *
 * Returns 0 if allowed, -1 if denied.
 */
int security_check_mem(unsigned long address, int size);

#endif /* BASICPP_SECURITY_H */
