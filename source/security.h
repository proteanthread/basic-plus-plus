 // ---
 // BASIC++ Interpreter - security.h
 // ---
 //
 // Security system interface.
 //
 // PURPOSE:
 // Enforces capability-gated access control on sensitive operations.
 // Every file I/O, compilation, chain, and system-level operation
 // passes through security_check() before executing.
 //
 // 6-LEVEL SECURITY MODEL:
 // SEC_OPEN        (0) - No restrictions (default)
 // SEC_SAFE        (1) - Secure but functional
 // SEC_STANDARD    (2) - Controlled sandbox
 // SEC_EDUCATIONAL (3) - Classroom mode
 // SEC_RESTRICTED  (4) - Very limited
 // SEC_PARANOID    (5) - Pure computation only
 //
 // OPERATIONS:
 // Each sensitive operation has a SecOperation code. The permission
 // matrix maps (SecLevel x SecOperation) -> allowed/denied.
 //
 // ---

#ifndef BASICPP_SECURITY_H
#define BASICPP_SECURITY_H

// --- Security Levels ---
typedef enum SecLevel {
 SEC_OPEN = 0, // no restrictions
 SEC_SAFE = 1, // secure but functional
 SEC_STANDARD = 2, // controlled sandbox
 SEC_EDUCATIONAL = 3, // classroom mode
 SEC_RESTRICTED = 4, // very limited
 SEC_PARANOID = 5, // pure computation only
 SEC_COUNT = 6 // sentinel
} SecLevel;

// --- Securable Operations ---
typedef enum SecOperation {
    SECOP_FILE_READ = 0, // LOAD, BLOAD, MERGE, INPUT#
    SECOP_FILE_WRITE = 1, // SAVE, BSAVE, OPEN, PRINT#
    SECOP_FILE_MGMT = 2, // MKDIR, RMDIR, CHDIR, KILL, NAME
    SECOP_FILE_BLOCK = 3, // GET, PUT records
    SECOP_FILE_STREAM = 4, // Sequential stream I/O
    SECOP_COMPILE = 5, // COMPILE command
    SECOP_CHAIN = 6, // CHAIN command
    SECOP_SYSTEM = 7, // system-level operations
    SECOP_MODULE = 8, // module activation
    SECOP_USB = 9, // USB hardware module access
    SECOP_VDEV = 10, // Virtual device manipulation
    SECOP_VTERM = 11, // Terminal interceptors
    SECOP_VCON = 12, // Console output hijack
    SECOP_EVAL = 13, // Dynamic string execution (exec_line)
    SECOP_NETWORK = 14, // TCP/UDP Raw Sockets
    SECOP_MEM_READ = 15, // PEEK memory read
    SECOP_MEM_WRITE = 16, // POKE memory write
    SECOP_PROG_MGMT = 17, // LIST/LOAD/SAVE/RUN/NEW
    SECOP_EXT_LOAD = 18, // LOAD FUNCTION/LIBRARY/MODULE
    SECOP_COUNT = 19 // sentinel
} SecOperation;

// --- Security API ---

void security_init(SecLevel level);
SecLevel security_get_level(void);
void security_set_level(SecLevel level);
const char *security_level_name(SecLevel level);
int security_find_level_by_name(const char *name);
int security_check(SecOperation op, int line_num);
int security_module_allowed(unsigned int capabilities);
int security_check_mem(unsigned long address, int size);
int security_check_port(int port, int line_num);
int security_check_pinned_level(SecLevel required_level);
int security_check_path(const char *path, int line_num);

// SECURITY RESTRICT API
int security_restrict_op(SecOperation op);
int security_is_op_restricted(SecOperation op);
int security_restrict_keyword(int kw_id);
int security_is_keyword_restricted(int kw_id);
void security_restrict_list(void);
int security_restrict_count(void);

#endif // BASICPP_SECURITY_H
