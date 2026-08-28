# `LOCK` File Range Locking Statement

## 1. BASIC Usage and Keyword Definition

Locks a record range or byte region in an open file channel to prevent concurrent process modification.

### Syntax Signatures:
```basic
LOCK [#]filenum% [, [start_record&] TO end_record&]
```

### Error Handling & Boundary Conditions:
- **Error 70 (ERR_PERMISSION_DENIED)**: Record already locked by another process.

### Operational Notes:
- Multi-process safe record locking.

---

## 2. Code Examples

```basic
10 OPEN "R", #1, "DATA.DAT", 128
20 LOCK #1, 10 TO 20 : REM Lock records 10..20
30 REM Perform atomic updates
40 UNLOCK #1, 10 TO 20
50 CLOSE #1
```
