# `BASEDIR$` Directory Path Extraction Function

## 1. BASIC Usage and Function Definition

The `BASEDIR$` function parses a full file path string and returns the directory path component (excluding the final filename component).

### Syntax Signatures:
```basic
dir_path$ = BASEDIR$(full_path$)
```

### Operational Rules:
- Handles both Windows (`\`) and POSIX (`/`) directory separators.
- Returns empty string or `"."` if no directory component is present.
- Memory safe: returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 P$ = "C:\Users\Admin\Projects\app.bas"
20 PRINT "Base Directory: "; BASEDIR$(P$) : REM Outputs "C:\Users\Admin\Projects"
```
