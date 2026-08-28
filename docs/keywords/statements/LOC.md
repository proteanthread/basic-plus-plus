# `LOC` File Channel Current Pointer Position Function

## 1. BASIC Usage and Keyword Definition

Returns the current byte offset or record position within an open file channel.

### Syntax Signatures:
```basic
position& = LOC(filenum%)
```

### Error Handling & Boundary Conditions:
- **Error 52 (ERR_BAD_FILE_NUMBER)**: File channel is not open.

### Operational Notes:
- Returns byte offset in sequential/binary files, record number in random files.

---

## 2. Code Examples

```basic
10 OPEN "R", #1, "DATA.DAT", 64
20 GET #1, 5
30 PRINT "Current Record: "; LOC(1) : REM Outputs 5
40 CLOSE #1
```
