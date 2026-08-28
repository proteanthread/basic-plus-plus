# C17 API Reference: Core Boot Subsystem (`core/boot.h`)

## 1. Subsystem Overview & Responsibilities

The Core Boot Subsystem (`core/boot.h`, implemented in `engine/src/core/boot.c`) coordinates the deterministic 9-phase initialization sequence, memory pool allocations, virtual machine instantiation, and graceful teardown for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Deterministic 9-Phase Boot Sequence**:
  1. Platform Abstraction Initialization (`libplatform`).
  2. Memory Pool & Allocator Setup (`MemoryContext`).
  3. String Pool Instantiation (`StringContext`).
  4. Variable Table Initialization (`VariableContext`).
  5. Virtual Device Bus Creation (`VDevContext`, `VConContext`).
  6. Virtual Machine Core Instantiation (`VMContext`).
  7. Lexer & AST Parser Binding (`LexerContext`, `EvalContext`).
  8. Micro-Library & Statement Table Registration (`stmt_table_register_all()`).
  9. Dialect Profile & Environment Readying.
- **Target Memory Pool Invariants**:
  - `baspp` (Standard Desktop Edition): 640 MB (`671088640L` bytes).
  - `bpp` (Lite REPL Edition): 384 MB (`402653184L` bytes).
  - `bs` (Batch Script Runner): 64 MB (`67108864L` bytes).
- **Graceful Reverse Teardown**: Deallocates subsystems in exact reverse sequence, flushing all streams and preventing resource leaks.

## 2. Header Inclusion & Prerequisites

```c
#include "core/boot.h"
```

## 3. Data Structures & Types

```c
/* Aggregated Context Block Populated During Boot */
typedef struct {
    MemoryContext   *mem;
    StringContext   *str;
    VariableContext *var;
    VDevContext     *vdev;
    VMContext       *vm;
} BootContext;

/* Boot Configuration Parameters */
typedef struct {
    size_t prog_mem;        /* Memory allocated for program line text */
    size_t var_mem;         /* Memory allocated for variable table */
    size_t str_mem;         /* Memory allocated for reference-counted string pool */
    size_t scratch_mem;     /* Scratch evaluation buffer size */
    bool   is_repl;         /* true for interactive prompt, false for batch */
    bool   sdl_gui;         /* Enable desktop SDL2 graphics window */
    bool   sdl_ondemand;    /* Delay-load SDL2 when SCREEN/GRAPHICS is invoked */
} BootConfig;
```

## 4. Function Prototypes & Operational Contracts

### Primary Boot Sequence
```c
/**
 * @brief Runs the deterministic 9-phase boot sequence using a custom BootConfig.
 * @param config Pointer to populated BootConfig struct.
 * @return Populated BootContext pointer on success, or NULL on boot failure.
 */
BootContext *boot_execute(const BootConfig *config);

/**
 * @brief Shuts down all boot subsystems in exact reverse order of initialization.
 */
void boot_shutdown(BootContext *ctx);
void boot_shutdown_ex(BootContext *ctx, bool force_exit);
```

### High-Level Helpers
```c
/**
 * @brief Boots the BASIC++ engine with default heap allocation.
 * @param heap_size Total memory pool size in bytes (e.g. 671088640L for baspp).
 * @return Initialized VMContext pointer ready for program execution.
 */
VMContext *boot_system(size_t heap_size);

/**
 * @brief Shuts down the engine given a VMContext pointer.
 */
void boot_shutdown_vm(VMContext *vm);
```

## 5. Architectural Invariants & Memory Discipline

- **Memory Partitioning**: `boot_system()` automatically partitions the requested heap size into program memory (20%), variable memory (20%), string heap (50%), and scratch memory (10%).
- **Opt-In Logging**: Boot initialization must never create log files on disk by default unless `--log` is explicitly passed.

## 6. Code Example: Booting Engine, Running Script, and Shutting Down

```c
#include "core/boot.h"
#include "vm/vm.h"

int main(void) {
    /* Boot standard 640 MB desktop engine */
    VMContext *vm = boot_system(671088640L);
    if (!vm) {
        return 1;
    }

    /* Execute BASIC statements */
    BppError err = vm_execute_line(vm, "PRINT \"BASIC++ Engine Initialized Successfully\"");

    /* Clean shutdown */
    boot_shutdown_vm(vm);
    return 0;
}
```
