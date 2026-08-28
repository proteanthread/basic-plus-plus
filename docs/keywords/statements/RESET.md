# `RESET` Close All Files and Flush Buffers Statement

## 1. BASIC Usage and Keyword Definition

Closes all open file channels and flushes all pending file system disk buffers.

### Syntax Signatures:
```basic
RESET
```

### Operational Notes:
- Safe bulk file channel closer.

---

## 2. Code Examples

```basic
10 OPEN "O", #1, "FILE1.DAT"
20 OPEN "O", #2, "FILE2.DAT"
30 RESET : REM Closes channels 1 and 2
```
