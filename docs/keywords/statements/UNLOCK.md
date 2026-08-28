# `UNLOCK` File Record Range Unlocking Statement

## 1. BASIC Usage and Keyword Definition

Releases record locks previously acquired via LOCK on an open file channel.

### Syntax Signatures:
```basic
UNLOCK [#]filenum% [, [start_record&] TO end_record&]
```

### Error Handling & Boundary Conditions:
- **Error 70 (ERR_PERMISSION_DENIED)**: Range was not locked by this process.

### Operational Notes:
- Multi-process record locking synchronization.

---

## 2. Code Examples

```basic
10 OPEN "R", #1, "DATA.DAT", 128
20 LOCK #1, 5 TO 10
30 REM Perform atomic modifications
40 UNLOCK #1, 5 TO 10
50 CLOSE #1
```
