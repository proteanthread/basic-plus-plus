# `POINTER` File Record Byte Position Query Function

## 1. BASIC Usage and Keyword Definition

Returns the exact 64-bit byte seek position within an open file channel.

### Syntax Signatures:
```basic
byte_pos& = POINTER(filenum%)
```

### Error Handling & Boundary Conditions:
- **Error 52 (ERR_BAD_FILE_NUMBER)**: Channel is not open.

### Operational Notes:
- High-precision seek pointer query.

---

## 2. Code Examples

```basic
10 OPEN "B", #1, "DATA.BIN"
20 SEEK #1, 1024
30 PRINT "Pointer position: "; POINTER(1) : REM Outputs 1024
40 CLOSE #1
```
