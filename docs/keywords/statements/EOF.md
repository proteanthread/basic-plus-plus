# `EOF` End of File Detection Function

## 1. BASIC Usage and Keyword Definition

Returns -1 (TRUE) if the end of the specified file channel has been reached, or 0 (FALSE) otherwise.

### Syntax Signatures:
```basic
is_eof% = EOF(filenum%)
```

### Error Handling & Boundary Conditions:
- **Error 52 (ERR_BAD_FILE_NUMBER)**: Channel is not open.

### Operational Notes:
- Essential for sequential file reading loops.

---

## 2. Code Examples

```basic
10 OPEN "I", #1, "DATA.TXT"
20 WHILE NOT EOF(1)
30   LINE INPUT #1, L$
40   PRINT L$
50 WEND
60 CLOSE #1
```
