/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: security_core.h
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
 * File: security_core.h
 * ===================================================================== */

#ifndef STANDALONE_SECURITY_CORE_H
#define STANDALONE_SECURITY_CORE_H

typedef enum SecLevel {
    SEC_OPEN = 0,
    SEC_SAFE = 1,
    SEC_STANDARD = 2,
    SEC_EDUCATIONAL = 3,
    SEC_RESTRICTED = 4,
    SEC_PARANOID = 5,
    SEC_COUNT = 6
} SecLevel;

typedef enum SecOperation {
    SECOP_FILE_READ = 0,
    SECOP_FILE_WRITE = 1,
    SECOP_FILE_MGMT = 2,
    SECOP_FILE_BLOCK = 3,
    SECOP_FILE_STREAM = 4,
    SECOP_COMPILE = 5,
    SECOP_CHAIN = 6,
    SECOP_SYSTEM = 7,
    SECOP_MODULE = 8,
    SECOP_USB = 9,
    SECOP_VDEV = 10,
    SECOP_VTERM = 11,
    SECOP_VCON = 12,
    SECOP_EVAL = 13,
    SECOP_NETWORK = 14,
    SECOP_MEM_READ = 15,
    SECOP_MEM_WRITE = 16,
    SECOP_PROG_MGMT = 17,
    SECOP_EXT_LOAD = 18,
    SECOP_COUNT = 19
} SecOperation;

void security_core_init(SecLevel level);
SecLevel security_core_get_level(void);
void security_core_set_level(SecLevel level);
const char *security_core_level_name(SecLevel level);
int security_core_find_level_by_name(const char *name);
int security_core_check_op(SecLevel level, SecOperation op);
int security_core_check_path(SecLevel level, const char *path);
int security_core_check_port(SecLevel level, int port);

/* RESTRICT API */
int security_core_restrict_op(SecOperation op, int restricted);
int security_core_is_op_restricted(SecOperation op);
int security_core_restrict_keyword(int kw_id, int restricted);
int security_core_is_keyword_restricted(int kw_id);
int security_core_restrict_count(void);
void security_core_restrict_clear(void);

#endif /* STANDALONE_SECURITY_CORE_H */
