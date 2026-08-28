# C17 API Reference: Dynamic Module Subsystem (`module/module.h`)

## 1. Subsystem Overview & Responsibilities

The Dynamic Module Subsystem (`module/module.h`, implemented in `engine/src/module/module.c`) provides dynamic loading, capability verification, sandbox allocation, registration, and activation for native C17 extensions and shared libraries (`.dll` / `.so`) in BASIC++ v6.5.2.

Key architectural responsibilities include:
- **Auditable & Safe Plugin Pipeline**: Modules load through a strict multi-phase pipeline: Validation $\rightarrow$ Capability Verification $\rightarrow$ Sandbox Allocation $\rightarrow$ Registration $\rightarrow$ Activation. Modules must NEVER directly modify VM instructions, bypass security gates, or corrupt internal stacks.
- **Module Classification (`BppModuleClass`)**:
  - `MOD_LIBRARY`: Shared procedural or function library (e.g. math extensions, crypto).
  - `MOD_DIALECT`: Language dialect syntax provider (e.g. ECMA-116, QBASIC).
  - `MOD_DEVICE`: Virtual hardware or peripheral device driver.
  - `MOD_EXTENSION`: General-purpose system integration plugin.
- **Granular Capability Bitmasks**: Modules declare required capability flags (`CAP_MATH`, `CAP_STRING`, `CAP_IO`, `CAP_FILE`, `CAP_SYSTEM`, `CAP_GRAPHICS`, `CAP_NETWORK`, `CAP_GPIO`, `CAP_USB`).
- **Dynamic Activation & Teardown**: Binds exported statement and function symbols into `FunctionRegistry` and `StmtTable`, executing module cleanup hooks on `UNLOAD`.

## 2. Header Inclusion & Prerequisites

```c
#include "module/module.h"
#include "security/security.h"
#include "types/errors.h"
```

## 3. Data Structures & Types

```c
#define MAX_MODULES 16

typedef enum {
    MOD_LIBRARY   = 0,
    MOD_DIALECT   = 1,
    MOD_DEVICE    = 2,
    MOD_EXTENSION = 3
} BppModuleClass;

/* Capability Flags */
#define CAP_NONE      0x0000u
#define CAP_MATH      0x0001u
#define CAP_STRING    0x0002u
#define CAP_IO        0x0004u
#define CAP_FILE      0x0008u
#define CAP_SYSTEM    0x0010u
#define CAP_GRAPHICS  0x0020u
#define CAP_SOUND     0x0040u
#define CAP_NETWORK   0x0080u
#define CAP_GPIO      0x0100u
#define CAP_USB       0x4000u

/* Module Information Descriptor */
typedef struct {
    const char     *name;           /* Module identifier (e.g. "SQLITE", "PHYSICS_2D") */
    const char     *version;        /* Version string (e.g. "1.0.0") */
    const char     *description;    /* Human-readable summary */
    BppModuleClass  mod_class;      /* MOD_LIBRARY, MOD_DEVICE, etc. */
    unsigned int    capabilities;   /* Bitmask of required CAP_* flags */
    BppSecLevel     required_level; /* Minimum security level permitted */
    int           (*init)(void *);  /* Initialization callback */
    void          (*cleanup)(void); /* Teardown callback */
} BppModuleInfo;
```

## 4. Function Prototypes & Operational Contracts

```c
/**
 * @brief Initializes the module subsystem and dynamic loader tables.
 */
void module_system_init(void);

/**
 * @brief Registers a new module descriptor with the system.
 */
int module_register(const BppModuleInfo *info);

/**
 * @brief Activates a registered module, invoking its init() hook and binding symbols.
 */
int module_activate(const char *name, void *rt);

/**
 * @brief Deactivates an active module, invoking cleanup() and unregistering symbols.
 */
int module_deactivate(const char *name);

/**
 * @brief Checks if a module is currently active.
 */
int module_is_active(const char *name);

/**
 * @brief Finds module descriptor by name.
 */
const BppModuleInfo *module_find(const char *name);
```

## 5. Architectural Invariants

- **Capability Verification Gate**: If `module->capabilities` exceeds the permissions granted by the active `SecurityContext`, `module_activate()` aborts with Error 70 (`ERR_PERMISSION_DENIED`).
- **Bounded Table**: Maximum concurrent loaded modules is bounded by `MAX_MODULES` (16).

## 6. Code Example: Writing a C17 Module for BASIC++

```c
#include "module/module.h"
#include <stdio.h>

static int mymod_init(void *rt) {
    printf("MyMod Initialized successfully!\n");
    return 0;
}

static void mymod_cleanup(void) {
    printf("MyMod Cleaned up.\n");
}

static const BppModuleInfo g_mymod_info = {
    .name = "MYMOD",
    .version = "1.0.0",
    .description = "Sample Extension Module",
    .mod_class = MOD_EXTENSION,
    .capabilities = CAP_MATH | CAP_STRING,
    .required_level = SEC_STANDARD,
    .init = mymod_init,
    .cleanup = mymod_cleanup
};

void register_mymod(void) {
    module_register(&g_mymod_info);
}
```
