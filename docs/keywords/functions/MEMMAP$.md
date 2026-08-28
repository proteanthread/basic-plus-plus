# `MEMMAP$` Memory Layout Diagnostic String Function

## 1. BASIC Usage and Function Definition

The `MEMMAP$` function returns a formatted multi-line ASCII diagnostic string displaying the active memory layout, segmented memory allocations, bank mappings, string pool utilization, and variable table metrics of the current BASIC++ VM process.

### Syntax Signatures:
```basic
map$ = MEMMAP$()
map$ = MEMMAP$(selector%)
```

### Operational Rules:
- **`MEMMAP$()` / `MEMMAP$(0)`**: Returns the complete global memory map overview (total pool, used heap, string pool, segmented memory, bank count).
- **`MEMMAP$(1)`**: Returns segmented memory region diagnostics (64KB banks, segment registers, VRAM buffers).
- **`MEMMAP$(2)`**: Returns variable table metrics (scalar counts, active arrays, allocated elements).
- **String Memory**: Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 PRINT "=== BASIC++ Virtual Memory Map ==="
20 PRINT MEMMAP$()
```
