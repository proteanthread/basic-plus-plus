<!--
Title:        Module Guide
Tier:         1
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot, bppc, trans, detok)
Authority:    engine/src/module/module.c, engine/include/module/module.h
Generated:    no
Status:       Active
-->

# BASIC++ v6.5.2 Modular Programming & Module System Guide

The comprehensive architectural reference and programmer manual for modular programming, namespace encapsulation, runtime-loadable C modules, and the 5-stage module security pipeline.

## 1. Modular Programming & Scoping in BASIC++

BASIC++ provides a first-class modular programming system conforming to the ECMA-116 standard. Modules eliminate global namespace pollution, establish encapsulation boundaries, and enable code reuse across projects.

### 1.1 Declaring Modules
Modules are declared using `MODULE ... END MODULE` blocks:

```basic
MODULE MathUtils
    PUBLIC CONST PI_2 = 6.283185307179586
    PRIVATE cache_val = 0

    PUBLIC FUNCTION DoubleVal(x)
        DoubleVal = x * 2
    END FUNCTION
END MODULE
```

- **Default Privacy**: All variables, constants, procedures, and types defined within a module default to private scope unless explicitly prefixed with `PUBLIC`.
- **Private Encapsulation**: Private members (`cache_val`) cannot be accessed or modified from outside the declaring module.

### 1.2 Importing Modules
Client programs import modules using the `IMPORT` statement:

```basic
IMPORT MathUtils
PRINT MathUtils.DoubleVal(21)
```

- **Namespace Aliasing**: Modules can be renamed upon import (`IMPORT MathUtils AS M`), allowing short prefixes (`M.DoubleVal(21)`).
- **Selective Unqualified Imports**: Specific symbols can be imported directly into the local lexical scope using `FROM ... IMPORT`:

```basic
FROM MathUtils IMPORT DoubleVal, PI_2
PRINT DoubleVal(10)
PRINT "Tau: "; PI_2
```

### 1.3 Module Lifecycle Hooks
Modules may declare automatic startup and shutdown blocks:

```basic
MODULE DataStore
    PUBLIC SUB Fetch(id%)
        PRINT "Fetching record: "; id%
    END SUB

    INITIALIZE
        PRINT "DataStore initialized."
    TERMINATE
        PRINT "DataStore resources cleaned up."
END MODULE
```

---

## 2. Runtime C Module Loader

BASIC++ supports runtime-loadable extension modules (`.dll` on Windows, `.so` on Linux) that introduce new keywords, statement handlers, and virtual devices without modifying the core engine.

### 2.1 Management Commands
- `MODULE LOAD "name"`: Locates and dynamically loads an extension module from the module search path.
- `MODULE UNLOAD "name"`: Deactivates a module and unbinds its keywords.
- `MODULE LIST`: Lists all active modules, their versions, and declared capability sets.
- `MODULE INFO "name"`: Displays detailed metadata for a loaded module.

### 2.2 Built-In Dynamic Modules
BASIC++ includes three standard pre-compiled modules:
1. **`mathext` (`engine/src/module/mathext.c`)**: Advanced functions including `GAMMA`, `BETA`, `ERF`, and Bessel functions.
2. **`arrayext` (`engine/src/module/arrayext.c`)**: High-performance array operations: `ARRAY SORT`, `ARRAY REVERSE`, `ARRAY SHUFFLE`, `ARRAY SEARCH`, `FILTER`, `REDUCE`.
3. **`regex` (`engine/src/module/regex.c`)**: Regular expression pattern matching via `REGEX.MATCH` and `REGEX.REPLACE`.

---

## 3. Five-Stage Module Security Pipeline

Modules loaded at runtime pass through a strict security pipeline:

1. **Validation**: The module binary header is verified for version compatibility, C17 ABI alignment, and checksum integrity.
2. **Capability Verification**: The module's declared capabilities (file access, network sockets, raw memory, device creation) are checked against the active security level via `security_module_allowed()`. Modules requesting denied capabilities are rejected.
3. **Sandbox Allocation**: A restricted `VMContext` sandbox is created, preventing modules from corrupting VM execution stacks or modifying core instructions.
4. **Registration**: Module keywords and functions are assigned unique IDs and registered with `StmtRegistry` and `FuncRegistry`.
5. **Activation**: The module's initialization routine runs, and its symbols become active in the parser.

---

## 4. Custom C17 Module Development

External modules implement the `BppModuleInterface` struct:

```c
#include "module/module.h"

static BppError my_module_init(VMContext* ctx) {
    // Register custom statements and functions
    return BPP_OK;
}

static void my_module_shutdown(VMContext* ctx) {
    // Release allocated memory and handles
}

static BppModuleInterface my_module_desc = {
    .name         = "custom_io",
    .version      = "1.0.0",
    .capabilities = BPP_CAP_FILE_IO,
    .init         = my_module_init,
    .shutdown     = my_module_shutdown,
};
```
