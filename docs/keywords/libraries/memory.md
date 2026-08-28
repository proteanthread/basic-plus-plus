# `memory` Dynamic Memory Management Subsystem (`libkernel`)

## 1. Architectural Purpose & Overview

The `memory` subsystem (`engine/src/memory/memory.c`) manages heap allocation pools, program line buffers, and garbage collection in BASIC++ v6.5.2.

### Key Architectural Invariants:
- **Standard Memory Footprints**:
  - `baspp` (Desktop Standard): 640 MB (`671088640L` bytes)
  - `bpp` (Lite REPL): 384 MB (`402653184L` bytes)
  - `bs` (Batch Runner): 64 MB (`67108864L` bytes)
- **Zero-Initialization**: Allocations are zero-initialized via `calloc` or `memset`.
- **64-bit Pointer Safety**: Uses `uintptr_t` / `intptr_t` for pointer calculations.

---

## 2. Technical API Signatures (C17)

```c
MemContext *mem_create(size_t capacity);
void mem_destroy(MemContext *mem);
void *mem_alloc(MemContext *mem, size_t size);
void mem_free(MemContext *mem, void *ptr);
size_t mem_get_free_ram(MemContext *mem);
```
