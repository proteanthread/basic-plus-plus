# C17 API Reference: Memory Management & Program Storage (`memory/memory.h`)

## 1. Subsystem Overview & Responsibilities

The Memory Management Subsystem (`memory/memory.h`, implemented in `engine/src/memory/memory.c`) provides partitioned memory pools, an ephemeral scratch arena, and ordered program line storage for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Partitioned Memory Pools**: Allocates and manages bounded pools for program storage (`prog_mem`), variable tables (`var_mem`), string heap (`str_mem`), and scratch evaluation arena (`scratch_mem`).
- **Ephemeral Scratch Arena**: Provides fast, non-fragmenting temporary memory allocation (`mem_scratch_alloc()`) reset automatically after statement execution (`mem_scratch_reset()`).
- **Numbered Program Line Storage**: Stores canonical BASIC program lines ordered by line number (0 to 65535), supporting insertion, replacement, deletion, and fast sequential iteration (`mem_program_get_all()`).
- **Memory Footprint Standards**:
  - `baspp` (Standard Desktop Edition): 640 MB (`671088640L` bytes).
  - `bpp` (Lite REPL Edition): 384 MB (`402653184L` bytes).
  - `bs` (Batch Script Runner): 64 MB (`67108864L` bytes).

## 2. Header Inclusion & Prerequisites

```c
#include "memory/memory.h"
#include "types/types.h"
```

## 3. Data Structures & Types

```c
/* Opaque Memory Context */
typedef struct MemoryContext MemoryContext;

/* Structure Representing a Stored Program Line */
typedef struct {
    BppLineNumber line_number; /* 16-bit line number (1..65529) */
    char         *text;        /* Null-terminated line text */
} BppProgramLine;
```

## 4. Function Prototypes & Operational Contracts

### Memory Context Lifecycle
```c
/**
 * @brief Initializes the MemoryContext with partitioned pool sizes.
 * @return Allocated MemoryContext pointer, or NULL on OOM.
 */
MemoryContext *mem_init(size_t prog_mem_sz, size_t var_mem_sz, size_t str_mem_sz, size_t scratch_mem_sz);

/**
 * @brief Frees all memory pools and deallocates the MemoryContext.
 */
void mem_shutdown(MemoryContext *ctx);
```

### Ephemeral Scratch Arena
```c
/**
 * @brief Allocates temporary memory from the scratch arena.
 * @note Scratch memory is automatically reset after statement/command execution.
 */
void *mem_scratch_alloc(MemoryContext *ctx, size_t size);

/**
 * @brief Resets the scratch arena, invalidating all ephemeral allocations.
 */
void mem_scratch_reset(MemoryContext *ctx);
```

### Program Line Management
```c
/**
 * @brief Inserts or replaces a program line in sorted order.
 */
bool mem_program_insert(MemoryContext *ctx, BppLineNumber line, const char *text);

/**
 * @brief Deletes a program line by line number.
 */
bool mem_program_delete(MemoryContext *ctx, BppLineNumber line);

/**
 * @brief Clears all stored program lines (NEW statement).
 */
void mem_program_clear(MemoryContext *ctx);

/**
 * @brief Retrieves program line text by line number.
 */
const char *mem_program_get(MemoryContext *ctx, BppLineNumber line);

/**
 * @brief Retrieves contiguous array of all stored program lines.
 * @param out_count Populated with total number of stored lines.
 */
BppProgramLine *mem_program_get_all(MemoryContext *ctx, size_t *out_count);

/**
 * @brief Returns the next line number following current_line.
 */
bool mem_program_next_line(MemoryContext *ctx, BppLineNumber current_line, BppLineNumber *next_line);
```

## 5. Architectural Invariants

- **Canonical Source Preservation**: Stored program lines preserve exact original source text formatting (ephemeral tokenization).
- **Sorted Ordering**: Program lines are strictly sorted in ascending numerical order for deterministic execution and `LIST` output.

## 6. Code Example: Storing and Iterating Program Lines

```c
#include "memory/memory.h"
#include <stdio.h>

void load_mini_program(MemoryContext *mem) {
    mem_program_insert(mem, 10, "PRINT \"Line 10\"");
    mem_program_insert(mem, 20, "GOTO 10");

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    for (size_t i = 0; i < count; i++) {
        printf("%u %s\n", lines[i].line_number, lines[i].text);
    }
}
```
