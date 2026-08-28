# `FILESIZE` File Size Query Function

## 1. BASIC Usage and Keyword Definition

Returns the total size of the specified file in bytes.

### Syntax Signatures:
```basic
bytes& = FILESIZE(filename$)
```

### Error Handling & Boundary Conditions:
- **Error 53 (ERR_FILE_NOT_FOUND)**: File does not exist.

### Operational Notes:
- Returns 64-bit integer file length.

---

## 2. Code Examples

```basic
10 PRINT "File size: "; FILESIZE("ARCHIVE.ZIP"); " bytes"
```
