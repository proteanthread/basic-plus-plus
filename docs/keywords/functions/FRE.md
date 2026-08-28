# `FRE` Free Memory Query Function

## 1. BASIC Usage and Function Definition

The `FRE` function returns the number of bytes of available free memory in the active BASIC++ memory pool, or performs garbage collection on the reference-counted string pool.

### Syntax Signatures:
```basic
free_bytes& = FRE(0)
free_bytes& = FRE("")
free_bytes& = FRE(-1)
```

### Operational Rules:
- **`FRE(0)` / `FRE(numeric)`**: Returns total free heap memory in bytes available for variables, arrays, and program storage.
- **`FRE("")` / `FRE(string$)`**: Forces memory compaction / garbage collection of the string pool and returns available string pool memory.
- **`FRE(-1)`**: Returns segmented virtual memory free space.

---

## 2. Code Examples

```basic
10 PRINT "Available Memory: "; FRE(0); " bytes"
20 PRINT "String Pool Free: "; FRE(""); " bytes"
```
