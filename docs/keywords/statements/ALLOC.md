# `ALLOC` Dynamic Heap Memory Allocation Function

## 1. BASIC Usage and Function Definition

The `ALLOC` function allocates a block of uninitialized dynamic heap memory of the specified byte size from the interpreter's managed memory pool and returns a memory address pointer (`uintptr_t`).

### Syntax Signatures:
```basic
ptr% = ALLOC(byte_size&)
```

### Operational Rules:
- **Sandbox Security Gate**: In restricted sandbox modes (`SEC_STRICT` or `SEC_PARANOID`), dynamic raw heap allocation is prohibited and triggers Error 70 (`ERR_PERMISSION_DENIED`).
- **Memory Pool**: Memory is allocated from the active engine memory pool (640MB `baspp`, 384MB `bpp`, 64MB `bs`).
- **Deallocation Requirement**: Memory allocated via `ALLOC` MUST be explicitly released using `FREE(ptr)` or `DEALLOC(ptr)` to prevent leaks.
- **Null on Failure**: Returns `0` if allocation fails due to pool exhaustion.

---

## 2. Language Dialect & Compatibility

| Dialect | Syntax | Security Gate | Return Type |
|---|---|---|---|
| **GW-BASIC / BASICA** | *None* | N/A | N/A |
| **QuickBASIC / QBASIC** | *None* | N/A | N/A |
| **C / C17** | `malloc(size)` | OS dependent | `void*` |
| **BASIC++ (Master)** | `ALLOC(bytes)` | Security Sandboxed | Integer/Pointer |

---

## 3. Examples

```basic
10 POOL_SIZE& = 4096
20 PTR% = ALLOC(POOL_SIZE&)
30 IF PTR% = 0 THEN PRINT "Out of memory!": END
40 PRINT "Allocated 4KB buffer at address: "; HEX$(PTR%)
50 FREE(PTR%)
60 PRINT "Buffer released."
```
