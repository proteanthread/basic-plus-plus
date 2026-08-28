# `DUMP` Memory Hex Dump Diagnostic Statement

## 1. BASIC Usage and Keyword Definition

Prints a formatted hex and ASCII memory dump for low-level debugging and memory inspection.

### Syntax Signatures:
```basic
DUMP address& [, length%]
```

### Operational Notes:
- Safe read-only memory inspector bounded to allocated RAM pool.

---

## 2. Code Examples

```basic
10 DUMP 0, 64 : REM Dumps first 64 bytes of virtual memory
```
