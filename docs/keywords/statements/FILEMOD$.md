# `FILEMOD$` File Modification Formatted Timestamp Function

## 1. BASIC Usage and Keyword Definition

Returns the last modification date and time of a file as a formatted ISO 8601 string.

### Syntax Signatures:
```basic
t$ = FILEMOD$(filename$)
```

### Error Handling & Boundary Conditions:
- **Error 53 (ERR_FILE_NOT_FOUND)**: File does not exist.

### Operational Notes:
- Formatted as YYYY-MM-DD HH:MM:SS.

---

## 2. Code Examples

```basic
10 PRINT "Last modified: "; FILEMOD$("MAIN.BAS")
```
