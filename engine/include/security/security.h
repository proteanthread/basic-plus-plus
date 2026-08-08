/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file security.h
 * @brief Security sandbox and restriction registry API.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares security levels (OPEN to PARANOID) and sensitive operation codes.
 * - Why it exists: Enforces mandatory capability access controls to protect environments from malicious scripts.
 * - Why it works this way: It defines a standard matrix interface. Calls to security_check query
 *   active levels and any explicit keyword/operation overrides.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Security operation enum entries and diagnostic descriptions.
 * - What cannot be changed: The core level definitions and check parameters.
 * - What to expect: Denied operations print error context and return -1.
 * - What to do if something breaks: Verify level mappings and active restrict list counts.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Relies on platform memory checks for PEEK/POKE.
 * - Portability concerns: Fully standard C17.
 */

#ifndef SECURITY_H
#define SECURITY_H

typedef enum {
    SEC_OPEN = 0,        /* All operations permitted */
    SEC_SAFE = 1,        /* Secure but functional */
    SEC_STANDARD = 2,    /* Controlled sandbox */
    SEC_EDUCATIONAL = 3, /* Classroom mode */
    SEC_RESTRICTED = 4,  /* Very limited */
    SEC_PARANOID = 5,    /* Pure computation only */
    SEC_COUNT = 6
} BppSecLevel;

typedef enum {
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
} BppSecOperation;

void        security_init(BppSecLevel level);
BppSecLevel security_get_level(void);
void        security_set_level(BppSecLevel level);
const char *security_level_name(BppSecLevel level);
int         security_find_level_by_name(const char *name);
int         security_check(BppSecOperation op, int line_num);
int         security_module_allowed(unsigned int capabilities);
int         security_check_pinned_level(BppSecLevel required_level);
int         security_check_mem(unsigned long address, int size);
int         security_check_port(int port, int line_num);
int         security_check_path(const char *path, int line_num);
int         security_check_file_path(const char *path, int line_num);

int         security_restrict_op(BppSecOperation op);
int         security_is_op_restricted(BppSecOperation op);
int         security_restrict_keyword(int kw_id);
int         security_is_keyword_restricted(int kw_id);
void        security_restrict_list(void);
int         security_restrict_count(void);

#endif /* SECURITY_H */
