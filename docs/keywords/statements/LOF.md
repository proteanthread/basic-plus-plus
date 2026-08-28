# `LOF` Length of File Function

## 1. BASIC Usage and Keyword Definition

Returns the total length of an open file channel in bytes.

### Syntax Signatures:
```basic
length_bytes& = LOF(filenum%)
```

### Error Handling & Boundary Conditions:
- **Error 52 (ERR_BAD_FILE_NUMBER)**: Channel is not open.

### Operational Notes:
- Returns 64-bit integer byte count.

---

## 2. Code Examples

```basic
10 OPEN "I", #1, "README.TXT"
20 PRINT "File size: "; LOF(1); " bytes"
30 CLOSE #1
```
