# C17 API Reference: Segmented Memory & Real-Mode Simulation (`memory/segmented_mem.h`)

## 1. Subsystem Overview & Responsibilities

The Segmented Memory Subsystem (`memory/segmented_mem.h`, implemented in `engine/src/memory/segmented_mem.c`) provides 8086 real-mode 1MB segmented address space virtualization (`0x00000` to `0xFFFFF`), virtual segment registers (`DEF SEG`), variable address handles (`VARSEG`, `VARPTR`), and memory reading/writing (`PEEK`, `POKE`) for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **8086 Segmented Addressing**: Calculates physical 20-bit addresses via standard real-mode segment-offset arithmetic:
  $$\text{Physical Address} = (\text{DEF SEG} \times 16) + \text{Offset}$$
- **Virtual Variable Handle Registry**: Maps raw 32-bit/64-bit heap variable pointers into deterministic 16-bit virtual segment and offset pairs (`VARSEG`/`VARPTR`), allowing legacy GW-BASIC and QuickBASIC memory manipulation code to inspect variables without host pointer corruption.
- **Memory-Mapped Device Intercepts**: Coordinates with `libhardware` / `BusContext` to route `PEEK`/`POKE` calls targeting VRAM (`0xB800:0x0000`, `0xA000:0x0000`) and BIOS Data Area (`0x0040:0x0000`) to appropriate virtual device handlers.
- **Memory Safety Guarding**: Enforces address range containment and prevents unaligned host memory faults.

## 2. Header Inclusion & Prerequisites

```c
#include "memory/segmented_mem.h"
#include "runtime/variables.h"
```

## 3. Data Structures & Types

```c
/* Opaque Segmented Memory Context */
typedef struct VMemContext VMemContext;
```

## 4. Function Prototypes & Operational Contracts

### Lifecycle & Segment Configuration
```c
/**
 * @brief Initializes the segmented memory context linked to a variable table.
 */
VMemContext *vmem_init(VariableContext *var);

/**
 * @brief Shuts down the segmented memory context.
 */
void vmem_shutdown(VMemContext *ctx);

/**
 * @brief Sets the active segment base register (DEF SEG statement).
 * @param seg 16-bit segment value (e.g. 0xB800 for CGA text VRAM).
 */
void vmem_set_def_seg(VMemContext *ctx, uint16_t seg);

/**
 * @brief Retrieves the active DEF SEG register value.
 */
uint16_t vmem_get_def_seg(VMemContext *ctx);
```

### Variable Handle Registration & Resolution (`VARPTR` / `VARSEG`)
```c
/**
 * @brief Registers a variable pointer and returns a 32-bit virtual handle (segment:offset).
 */
uint32_t vmem_register_handle(VMemContext *ctx, BValue *val, bool is_string_data);

/**
 * @brief Resolves a 32-bit virtual handle back to its original BValue pointer.
 */
BValue *vmem_resolve_handle(VMemContext *ctx, uint32_t handle, bool *is_string_data);
```

### Memory Access Operations (`PEEK` / `POKE`)
```c
/**
 * @brief Reads an 8-bit byte from the active DEF SEG and 16-bit address offset (PEEK function).
 * @return 0 on success, Error 5 or Error 70 on violation.
 */
int vmem_peek(VMemContext *ctx, uint16_t address, uint8_t *out_val);

/**
 * @brief Writes an 8-bit byte to the active DEF SEG and 16-bit address offset (POKE statement).
 */
int vmem_poke(VMemContext *ctx, uint16_t address, uint8_t val);
```

## 5. Architectural Invariants

- **Host Pointer Isolation**: Host 64-bit pointers are NEVER exposed directly as integers to `PEEK`/`POKE`; they are mapped through virtual handles to ensure 64-bit safety and prevent memory corruption.
- **Excluded in Lite Target**: The segmented memory subsystem is included in `baspp` (Standard Edition) and omitted from `bpp` (Lite Edition) to reduce footprint.

## 6. Code Example: Manipulating Simulated Video Memory

```basic
10 DEF SEG = &HB800 : REM Select CGA Text Video Segment
20 POKE 0, ASC("A") : REM Write character 'A' at top-left
30 POKE 1, 14       : REM Set yellow on black color attribute
40 PRINT "First character in VRAM is: "; CHR$(PEEK(0))
50 DEF SEG          : REM Reset DEF SEG to default BASIC data segment
```
