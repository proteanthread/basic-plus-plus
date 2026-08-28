// FILENAME: security.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libkernel
// NEEDS: platform, memory
// Provides core logic and interface definitions for security within BASIC++.
//
// ---- Includes ----

#ifndef SECURITY_H
#define SECURITY_H

typedef enum {
    SEC_OPEN = 0,        // All operations permitted
    SEC_SAFE = 1,        // Secure but functional
    SEC_STANDARD = 2,    // Controlled sandbox
    SEC_EDUCATIONAL = 3, // Classroom mode
    SEC_RESTRICTED = 4,  // Very limited
    SEC_PARANOID = 5,    // Pure computation only
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

#endif // SECURITY_H
