# `SEEK` File Seek Position Statement and Function

## 1. BASIC Usage and Keyword Definition

Sets or returns the absolute byte or record position in an open file channel.

### Syntax Signatures:
```basic
SEEK [#]filenum%, position_bytes&
current_seek& = SEEK(filenum%)
```

### Error Handling & Boundary Conditions:
- **Error 52 (ERR_BAD_FILE_NUMBER)**: File channel is not open.

### Operational Notes:
- 1-based byte offset for binary files; record number for random files.

---

## 2. Code Examples

```basic
10 OPEN "B", #1, "DATA.BIN"
20 SEEK #1, 256 : REM Seek to byte offset 256
30 PRINT "Seek position: "; SEEK(1)
40 CLOSE #1
```
