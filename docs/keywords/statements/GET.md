# `GET` Random File Record or Screen Block Retrieval Statement

## 1. BASIC Usage and Keyword Definition

Reads a record from a random-access file, or captures a rectangular pixel block from the screen into an array.

### Syntax Signatures:
```basic
GET [#]filenum% [, record_number&]
GET (x1%, y1%)-(x2%, y2%), array%
```

### Error Handling & Boundary Conditions:
- **Error 52 (ERR_BAD_FILE_NUMBER)**: File channel not open.

### Operational Notes:
- Dual functionality: file record I/O and graphics blit capture.

---

## 2. Code Examples

```basic
10 OPEN "R", #1, "DATA.DAT", 128
20 GET #1, 10 : REM Read record 10
30 CLOSE #1
```
