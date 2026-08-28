# `segmented_mem` Segmented Memory Virtualization Subsystem (`libhardware`)

## 1. Architectural Purpose & Overview

The `segmented_mem` library (`engine/src/memory/segmented_mem.c`) provides address space isolation, memory mapping registers, and bounds checking for virtualized retro machine memory.

### Key Architectural Invariants:
- **Bank Protection**: Enforces permissions across segmented memory banks.
- **Direct PEEK/POKE Interface**: Supports `PEEK`, `POKE`, `PEEKB`, `POKEB`, and `MEMMAP$`.

---

## 2. Technical API Signatures (C17)

```c
SegmentedMemContext *segmem_create(size_t total_banks);
void segmem_destroy(SegmentedMemContext *ctx);
uint8_t segmem_peek(SegmentedMemContext *ctx, int bank, uint16_t offset);
void segmem_poke(SegmentedMemContext *ctx, int bank, uint16_t offset, uint8_t val);
```
