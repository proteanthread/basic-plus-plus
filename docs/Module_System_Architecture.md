# BASIC++ Module System & Extension Loading Architecture

**Version 4.5.0**

---

## 1. Overview

The BASIC++ Module System provides a secure, modular pipeline to extend the interpreter's built-in vocabularies, dialects, and virtual hardware interfaces without modifying the core Virtual Machine (VM). 

No extension is capable of changing the execution engine or core VM instruction definitions. The architecture guarantees a clear separation of concerns, strict sandboxing, and capability-based access control.

---

## 2. The Four Primary Module Classes

All modular additions belong to one of four isolated classes:

### 1. Standard Library Modules (`MOD_LIBRARY`)
*   **Purpose**: Extend standard built-in functions (e.g. advanced arithmetic, string parsing, custom utilities).
*   **Access Bounds**: Expose new high-level BASIC functions to the environment. Must interact with the VM solely via the secure Function Registry API.

### 2. Dialect Modules (`MOD_DIALECT`)
*   **Purpose**: Define keyword interpretation, token mapping rules, and syntax compatibility limits.
*   **Emulation Profiles**: GW-BASIC, Palo Alto Tiny BASIC, Commodore 64 BASIC, Sinclair, etc.
*   **Boundary Rules**: Dialects configure the Lexer and Parser levels and map dialect-specific keywords to library calls. They are strictly prohibited from modifying core memory-model rules or introducing new virtual machine opcodes.

### 3. Device Modules (`MOD_DEVICE`)
*   **Purpose**: Implement virtual device driver backends (console buffers, file channels, hardware registers, serial connections).
*   **Access Bounds**: Adhere strictly to the Virtual Device interface contract. Standard BASIC programs interact with these drivers using virtualized hardware calls like `PEEK` and `POKE` routed directly to internal address maps.

### 4. Extension Modules (`MOD_EXTENSION`)
*   **Purpose**: Provide host-dependent domain-specific libraries or hybrid plugins.
*   **Access Bounds**: Executable only through controlled runtime interfaces. Subject to strict capability verification at startup and execution time.

---

## 3. The Controlled Loading Pipeline

At boot time or when a BASIC program invokes `LOAD MODULE "filename.dll"`, modules pass through a strict verification pipeline:

```
[Module Source]
      ↓
[Validation]               ← Parse name, version, and dependency lists
      ↓
[Capability Verification]   ← Match requested capabilities against security sandbox
      ↓
[Sandbox Allocation]       ← Set up stack limits and static memory allocation
      ↓
[Registration]             ← Store in module table and load functions into global registry
      ↓
[Activation]               ← Trigger init() callback, ready for runtime execution
```

*   **Idempotency & Reloading**: The system fully supports dynamic runtime reloading. Invoking `UNLOAD MODULE` followed by `LOAD MODULE` on the same file path releases previously registered resources and re-runs the loader pipeline.

---

## 4. Environment Safety Modes

> [!WARNING]
> **PLANNED / FUTURE** — This feature is not yet implemented.

The interpreter restricts execution using three distinct runtime safety zones:

### 1. Strict Sandbox Mode (`STRICT SANDBOX`)
*   **Enforcement**: Triggered if the `--sandbox` CLI option is passed or if running in restricted environments.
*   **Restrictions**: No direct OS access, no dynamic code generation/execution (JIT is bypassed or restricted), no raw memory manipulation. All timing, file system, network, and random generator inputs are virtualized.

### 2. Controlled Extension Mode (`CONTROLLED EXTENSION`)
*   **Enforcement**: Standard environment for verified user libraries and device plugins.
*   **Restrictions**: Access to host system is mediated strictly through virtual device wrappers (`vdev.c`). Custom host heap memory allocations are prohibited; modules must request blocks via the VM.

### 3. Trusted Core Mode (`TRUSTED CORE`)
*   **Enforcement**: Reserved exclusively for built-in modules shipped directly with the BASIC++ compiler (e.g. STDLIB, FujiNet, UPNP).
*   **Access**: Full access to the interpreter's internal structural symbols and direct system layers.

---

## 5. Developer Implementation Guidelines

To maintain interpreter stability and absolute isolation, external developers must follow these strict requirements:

### Avoid Platform-Specific Shared Library Divergence
To prevent build system drift between Windows (`LoadLibrary`/`GetProcAddress`) and Posix/Linux (`dlopen`/`dlsym`), compile-time loading wrapper symbols are abstracted inside `source/core/platform.c`. Developers should compile shared modules using the cross-platform `BASIC_EXPORT` macro defined in `module_api.h`.

### Avoid Symbol Collisions
To prevent namespace overlap in the global function registry, developers must prefix all exported function names with their module name (e.g. `MYMOD_PRINT`, `MYMOD_ADD`). Soft runtime signature matching validates parameter types at runtime, preventing signature collisions during parse time.

### Avoid Raw Host Allocations (Memory Corruption Prevention)
Modules must never call standard C library allocations (`malloc`, `free`, `realloc`) directly. Instead, call the interpreter's managed allocator wrapper hooks:
```c
// Correct: allocates from the interpreter-managed heap
uint8_t *buffer = (uint8_t *)rt_malloc(rt, size);
```
This ensures all memory blocks are garbage-collected and fit within the designated memory safety pools.

### Avoid Modifying VM Opcodes & Stacks
Modules must never:
*   Inject new bytecode opcodes or alter execution states inside the VM core.
*   Access raw host OS memory addresses outside of virtualized registers.
*   Manipulate VM evaluation stacks directly.

---

## 6. Implementation Specifications

### 1. Declarative Module Verification
Statically define module structures via `ModuleInfo` descriptors:
```c
typedef struct ModuleInfo {
    const char *name;             // Scoped module identifier
    const char *version;          // Major.Minor format
    const char *description;      // Short description
    ModuleClass mod_class;        // MOD_LIBRARY, MOD_DIALECT, etc.
    unsigned int capabilities;    // CAP_ bitfield (e.g. CAP_MATH | CAP_STRING)
    int (*init)(void *);          // Init callback (receives RuntimeState*)
    void (*cleanup)(void);        // Cleanup callback
} ModuleInfo;
```

### 2. Capability Bitfield Verification
When loading a module, compare its declared capabilities bitfield against active sandboxing limits. If a module declares a capability that is blocked (e.g., `CAP_FILE` when the sandbox is active), the load pipeline fails immediately:
```c
if (!security_module_allowed(info->capabilities)) {
    // Capability denied by current security level
    return -1;
}
```

### 3. Global Function Registry
Exported functions register dynamically using the Function Registry. Arguments are validated at registration time via min/max argument counts:
```c
typedef struct FunctionEntry {
    const char     *name;        // Function name (e.g., "ABS")
    KeywordId       keyword;     // Corresponding keyword ID
    FuncCategory    category;    // FCAT_MATH, FCAT_USER, etc.
    FuncReturnType  ret_type;    // FRET_INT, FRET_FLOAT, etc.
    int             min_args;    // Minimum arguments
    int             max_args;    // Maximum arguments
    FuncSafety      safety;      // FSAFE_PURE ... FSAFE_SYSTEM
    int             overridable; // 1=dialect can replace
    FuncHandler     handler;     // C function pointer
    const char     *help_text;   // One-line description
} FunctionEntry;
```
During interpreter execution, the parser extracts arguments and the registry validates argument count before dispatching to the handler.

### 4. Virtual Device Interface Wrappers
Device modules map hardware address buffers to custom peripheral lines. Inside `vdev.c`, virtual PEEK and POKE calls route directly to module callbacks:
```c
// vdev.c dispatch wrapper
void vdev_write_register(uint32_t addr, uint8_t value) {
    if (addr >= DEVICE_IO_START && addr <= DEVICE_IO_END) {
        // Route to the active MOD_DEVICE handler
        device_module_write(addr, value);
    }
}
```

---

## 7. Performance Optimizations

### 1. Static Metadata Caching
To bypass expensive runtime string lookups during instruction loops, the parser pre-processes module symbols. When a program is parsed, string names are resolved into direct integer indexes in the function registry table. Subsequent `RUN` loops fetch the function directly via table offsets.

### 2. Linear Array Lookup
The Function Registry stores function records in a linear array. Lookups use sequential search, which is acceptable at fewer than 128 entries. If the registry grows significantly, a hash table or sorted array with binary search could be substituted.
