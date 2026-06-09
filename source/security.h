/*
 * =====================================================================
 * BASIC++ Interpreter - security.h
 * =====================================================================
 *
 * Security system interface (Phase 15).
 *
 * PURPOSE:
 *   Enforces capability-gated access control on sensitive operations.
 *   Every file I/O, compilation, chain, and system-level operation
 *   passes through security_check() before executing.
 *
 * SECURITY LEVELS:
 *   SEC_OPEN       - No restrictions (default, matches pre-Phase 15)
 *   SEC_STANDARD   - File I/O allowed, COMPILE/CHAIN/SYSTEM blocked
 *   SEC_RESTRICTED - Math/string only, all I/O blocked
 *
 * OPERATIONS:
 *   Each sensitive operation has a SecOperation code. The permission
 *   matrix maps (SecLevel x SecOperation) -> allowed/denied.
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#ifndef BASICPP_SECURITY_H
#define BASICPP_SECURITY_H

/* =====================================================================
 * Security Levels
 * =====================================================================
 */
typedef enum SecLevel {
    SEC_OPEN       = 0,   /* no restrictions */
    SEC_STANDARD   = 1,   /* file I/O yes, system ops no */
    SEC_RESTRICTED = 2,   /* math/string only */
    SEC_COUNT      = 3    /* sentinel */
} SecLevel;

/* =====================================================================
 * Securable Operations
 * =====================================================================
 */
typedef enum SecOperation {
    SECOP_FILE_READ  = 0,   /* LOAD, BLOAD, MERGE, INPUT# */
    SECOP_FILE_WRITE = 1,   /* SAVE, BSAVE, OPEN, PRINT# */
    SECOP_COMPILE    = 2,   /* COMPILE command */
    SECOP_CHAIN      = 3,   /* CHAIN command */
    SECOP_SYSTEM     = 4,   /* system-level operations */
    SECOP_MODULE     = 5,   /* module activation */
    SECOP_COUNT      = 6    /* sentinel */
} SecOperation;

/* =====================================================================
 * Security API
 * =====================================================================
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
 *   op       - the operation to check
 *   line_num - BASIC line number (for error messages)
 */
int security_check(SecOperation op, int line_num);

/*
 * security_module_allowed - Check if a module's capabilities
 *   are permitted under the current security level.
 *
 * Returns 1 if allowed, 0 if denied.
 */
int security_module_allowed(unsigned int capabilities);

#endif /* BASICPP_SECURITY_H */
