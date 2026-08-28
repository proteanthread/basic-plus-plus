# C17 API Reference: Security Sandbox Subsystem (`security/security.h`)

## 1. Subsystem Overview & Responsibilities

The Security Sandbox Subsystem (`security/security.h`, implemented in `engine/src/security/security.c`) provides capability-based sandboxing, multi-tier security levels, operation access verification, filesystem path sandboxing, and keyword restriction for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **6 Security Tiers (`BppSecLevel`)**:
  - `SEC_OPEN` (0): Unrestricted execution; all operations permitted.
  - `SEC_SAFE` (1): Standard security with guarded direct memory and port writes.
  - `SEC_STANDARD` (2): Controlled sandbox restricting filesystem access to project directories.
  - `SEC_EDUCATIONAL` (3): Classroom mode preventing shell execution (`SYSTEM`, `SHELL`) and external network connections.
  - `SEC_RESTRICTED` (4): Highly restricted mode disallowing disk writes and dynamic modules.
  - `SEC_PARANOID` (5): Pure computation mode; zero disk, network, or hardware I/O permitted.
- **Granular Operation Checking (`BppSecOperation`)**: Verifies discrete operations prior to execution (`SECOP_FILE_READ`, `SECOP_FILE_WRITE`, `SECOP_FILE_MGMT`, `SECOP_SYSTEM`, `SECOP_MODULE`, `SECOP_NETWORK`, `SECOP_MEM_READ`, `SECOP_MEM_WRITE`, `SECOP_PROG_MGMT`).
- **Dynamic Keyword Restriction**: Allows disabling individual statement keywords at runtime via `security_restrict_keyword()`.
- **Path & Memory Sandboxing**: Enforces path containment rules and prevents out-of-bounds `PEEK`/`POKE` memory accesses.

## 2. Header Inclusion & Prerequisites

```c
#include "security/security.h"
```

## 3. Data Structures & Types

```c
/* Security Levels */
typedef enum {
    SEC_OPEN        = 0, /* All operations permitted */
    SEC_SAFE        = 1, /* Secure but functional */
    SEC_STANDARD    = 2, /* Controlled sandbox */
    SEC_EDUCATIONAL = 3, /* Classroom mode */
    SEC_RESTRICTED  = 4, /* Very limited */
    SEC_PARANOID    = 5  /* Pure computation only */
} BppSecLevel;

/* Protected Security Operations */
typedef enum {
    SECOP_FILE_READ   = 0,
    SECOP_FILE_WRITE  = 1,
    SECOP_FILE_MGMT   = 2,
    SECOP_COMPILE     = 5,
    SECOP_CHAIN       = 6,
    SECOP_SYSTEM      = 7,
    SECOP_MODULE      = 8,
    SECOP_NETWORK     = 14,
    SECOP_MEM_READ    = 15,
    SECOP_MEM_WRITE   = 16,
    SECOP_PROG_MGMT   = 17
} BppSecOperation;
```

## 4. Function Prototypes & Operational Contracts

### Lifecycle & Level Management
```c
/**
 * @brief Initializes the security subsystem at a specific level.
 */
void        security_init(BppSecLevel level);
BppSecLevel security_get_level(void);
void        security_set_level(BppSecLevel level);
const char *security_level_name(BppSecLevel level);
```

### Operation & Capability Verification
```c
/**
 * @brief Checks if a specific security operation is permitted at the active security level.
 * @param op The security operation enum to check.
 * @param line_num Current BASIC line number for error diagnostics.
 * @return 0 if permitted, non-zero error code (Error 70) if denied.
 */
int security_check(BppSecOperation op, int line_num);

/**
 * @brief Checks if a module with specified capability flags can be activated.
 */
int security_module_allowed(unsigned int capabilities);

/**
 * @brief Verifies whether access to a physical memory address is allowed.
 */
int security_check_mem(unsigned long address, int size);

/**
 * @brief Verifies whether access to a hardware I/O port is allowed.
 */
int security_check_port(int port, int line_num);

/**
 * @brief Verifies whether access to a filesystem path is permitted under sandboxing.
 */
int security_check_path(const char *path, int line_num);
```

### Keyword Restriction APIs
```c
int  security_restrict_keyword(int kw_id);
int  security_is_keyword_restricted(int kw_id);
void security_restrict_list(void);
```

## 5. Architectural Invariants

- **Fail-Closed Design**: Any unauthorized operation immediately returns Error 70 (`ERR_PERMISSION_DENIED`) and halts execution.
- **Monotonic Elevation Lock**: Once initialized at a restrictive security level, the security level cannot be lowered from user BASIC scripts.

## 6. Code Example: Guarding System Calls in Statement Handlers

```c
#include "security/security.h"
#include "types/errors.h"

BppError stmt_dangerous_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    
    /* Enforce system security check */
    if (security_check(SECOP_SYSTEM, vm_get_current_line(vm)) != 0) {
        err.code = 70;
        err.message = "Permission denied: Operation prohibited by security sandbox";
        return err;
    }

    /* Proceed with privileged execution */
    return err;
}
```
