# `BASENAME$` Filename Extraction Function

## 1. BASIC Usage and Function Definition

The `BASENAME$` function extracts the filename component from a path string, optionally stripping a specified file extension.

### Syntax Signatures:
```basic
name$ = BASENAME$(full_path$)
name$ = BASENAME$(full_path$, suffix_to_remove$)
```

### Operational Rules:
- Strips leading directory components separated by `/` or `\`.
- If `suffix_to_remove$` is provided and matches the end of the filename, it is stripped.

---

## 2. Code Examples

```basic
10 P$ = "/var/log/system_audit.log"
20 PRINT "Filename: "; BASENAME$(P$)             : REM "system_audit.log"
30 PRINT "No ext  : "; BASENAME$(P$, ".log")      : REM "system_audit"
```
