# `CLOSE` File Channel Termination Statement

## 1. BASIC Usage and Keyword Definition

Flushes write buffers and closes one or more active file channels. If no arguments are given, closes all open channels.

### Syntax Signatures:
```basic
CLOSE [[#]filenum% [, [#]filenum%...]]
```

### Error Handling & Boundary Conditions:
- **Error 52 (ERR_BAD_FILE_NUMBER)**: Invalid file descriptor channel.

### Operational Notes:
- Safe to call with multiple channels separated by commas.

---

## 2. Code Examples

```basic
10 OPEN "O", #1, "TEST.DAT"
20 PRINT #1, "DATA"
30 CLOSE #1
```
