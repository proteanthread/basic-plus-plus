# `CLOCK$` Formatted Timestamp String Function

## 1. BASIC Usage and Function Definition

The `CLOCK$` function returns the current system date and time formatted as a complete ISO 8601 calendar timestamp string `"YYYY-MM-DD HH:MM:SS"`.

### Syntax Signatures:
```basic
stamp$ = CLOCK$
stamp$ = CLOCK$()
```

### Operational Rules:
- Returns an explicit 19-character timestamp `"YYYY-MM-DD HH:MM:SS"`.
- Uses local platform clock time.

---

## 2. Code Examples

```basic
10 PRINT "Event logged at: "; CLOCK$
```
