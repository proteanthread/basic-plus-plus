# `MEM` Memory Usage Function

## 1. BASIC Usage and Function Definition

The `MEM` function returns the total memory consumed by active variables, arrays, and execution frames in the current VM instance.

### Syntax Signatures:
```basic
used_bytes& = MEM()
used_bytes& = MEM(selector%)
```

### Operational Rules:
- **`MEM(0)` / `MEM()`**: Returns total allocated variable and array memory in bytes.
- **`MEM(1)`**: Returns total allocated program bytecode and line storage in bytes.
- **`MEM(2)`**: Returns total memory pool capacity (e.g. 640MB in `baspp`).

---

## 2. Code Examples

```basic
10 PRINT "Memory used by program: "; MEM(1); " bytes"
20 PRINT "Memory used by data   : "; MEM(0); " bytes"
30 PRINT "Total pool capacity   : "; MEM(2); " bytes"
```
