# `CLEAR` Memory and Variable Reset Statement

## 1. BASIC Usage and Keyword Definition

Clears all variables, arrays, and string pools, resetting memory allocations and call stack depth.

### Syntax Signatures:
```basic
CLEAR [, [mem_limit&] [, stack_size&]]
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Invalid memory limit or stack size.

### Operational Notes:
- Preserves loaded program lines while resetting runtime execution state.

---

## 2. Code Examples

```basic
10 LET A = 42 : DIM B(10)
20 CLEAR
30 PRINT "A is now: "; A : REM Prints 0
```
