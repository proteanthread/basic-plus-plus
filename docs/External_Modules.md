# BASIC++ v6.5.2 External Modules

## 1. THE MODULE SYSTEM

BASIC++ supports runtime-loadable modules that extend the language with new keywords, functions, and device drivers. Modules are compiled C17 shared libraries (.dll on Windows, .so on Linux) that register with the VM through a well-defined API.

The module system is implemented in engine/src/module/module.c and is part of the libkernel library.

## 2. LOADING AND UNLOADING MODULES

MODULE LOAD "name" loads a module by name. The module loader searches for the shared library file in the module search path:

```basic
10 MODULE LOAD "mathext"        ' Load extended math module
20 PRINT GAMMA(5)               ' Use function from module
30 MODULE UNLOAD "mathext"      ' Unload when done
```

MODULE LIST displays all loaded modules with their names, versions, and capabilities.

## 3. BUILT-IN MODULES

BASIC++ ships with several built-in modules:

| Module | Description | Library |
|--------|-------------|---------|
| mathext | Extended math functions (GAMMA, BETA, ERF, BESSEL) | libflex |
| arrayext | Extended array operations (SORT, SEARCH, FILTER, REDUCE) | libflex |
| regex | Regular expression matching | libserver |

Built-in modules are compiled into the executable and do not require separate .dll/.so files. They are loaded with the same MODULE LOAD syntax.

## 4. MODULE SECURITY PIPELINE

When MODULE LOAD is called, the module passes through a five-stage security pipeline:

1. **Validation** — The module file is checked for a valid module header and version compatibility.
2. **Capability Verification** — The module's declared capabilities (file access, network, memory) are compared against the current security level. Modules requesting capabilities denied by the security level cannot load.
3. **Sandbox Allocation** — The module receives an isolated context with restricted access to the VM's internal state.
4. **Registration** — Module-provided keywords, functions, and devices are registered in the statement registry, function table, and device bus.
5. **Activation** — The module's init function runs and the module becomes active.

## 5. MODULE API

Modules implement the BppModuleInterface:

```c
typedef struct {
    const char* name;
    const char* version;
    uint32_t capabilities;
    BppError (*init)(VMContext* ctx);
    void (*shutdown)(VMContext* ctx);
    BppModuleKeyword* keywords;
    int keyword_count;
    BppModuleFunction* functions;
    int function_count;
} BppModuleInterface;
```

The init function receives the VMContext and can register keywords, functions, and devices. The shutdown function cleans up when the module is unloaded.

## 6. MODULE CAPABILITIES

Modules declare their required capabilities as a bitmask:

| Capability | Bit | Description |
|------------|-----|-------------|
| CAP_COMPUTE | 0x01 | Pure computation (always allowed) |
| CAP_CONSOLE | 0x02 | Console I/O |
| CAP_FILE | 0x04 | File system access |
| CAP_NETWORK | 0x08 | Network access |
| CAP_MEMORY | 0x10 | Extended memory access |
| CAP_DEVICE | 0x20 | Hardware device access |
| CAP_SHELL | 0x40 | Shell/process execution |

## 7. WRITING A CUSTOM MODULE

A minimal custom module:

```c
#include "bpp_api.h"

static BppError my_func(VMContext* ctx) {
    BValue val = eval_expression(ctx);
    if (val.type == VAL_ERROR) return val.as.error;
    double result = val.as.number * 2;
    push_number(ctx, result);
    return (BppError){0};
}

static BppModuleFunction funcs[] = {
    {"DOUBLE", my_func, 1}
};

BppModuleInterface bpp_module = {
    .name = "example",
    .version = "1.0",
    .capabilities = CAP_COMPUTE,
    .functions = funcs,
    .function_count = 1
};
```

Compile as a shared library and place in the module search path.

## 8. MODULE SEARCH PATH

The module loader searches these locations in order:

1. The current working directory.
2. The directory containing the BASIC++ executable.
3. The BPPMODULES environment variable (colon-separated on Linux, semicolon-separated on Windows).
4. The system library path (/usr/lib/bpp/modules on Linux).

## 9. RESTRICTIONS

Modules cannot:
- Directly modify VM instructions or opcode dispatch.
- Access internal stacks (ForStack, GosubStack, etc.) directly.
- Execute host code bypasses outside the sandbox.
- Corrupt internal memory structures.

These restrictions are enforced by providing modules with a limited API surface rather than raw access to the VMContext internals.
