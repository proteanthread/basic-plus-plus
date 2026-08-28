# `MEMMAP` Memory Mapping Statement

## 1. BASIC Usage and Keyword Definition

The `MEMMAP` statement configures virtual memory map windows, maps physical or hardware virtual device buffers into user memory banks, or prints a diagnostic memory overview.

### Syntax Signatures:
```basic
MEMMAP
MEMMAP bank%, target_segment&, size_bytes&
```

### Operational Rules:
- When executed without arguments, outputs the active virtual memory structure and bank allocation table to the console.
- When given arguments, maps virtual device address spaces into the specified memory bank.

---

## 2. Code Examples

```basic
10 MEMMAP : REM Display current memory mapping table
```
