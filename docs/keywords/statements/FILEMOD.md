# `FILEMOD` File Modification Numeric Timestamp Function

## 1. BASIC Usage and Keyword Definition

Returns the last modification timestamp of a file as a 64-bit Unix epoch integer.

### Syntax Signatures:
```basic
t# = FILEMOD(filename$)
```

### Error Handling & Boundary Conditions:
- **Error 53 (ERR_FILE_NOT_FOUND)**: File does not exist.

### Operational Notes:
- Facilitates fast modification date comparisons.

---

## 2. Code Examples

```basic
10 PRINT "Epoch timestamp: "; FILEMOD("DATA.DAT")
```
