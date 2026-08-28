# `TIME` / `TIME$` System Time Function & Variable

## 1. BASIC Usage and Keyword Definition

The `TIME$` variable/function returns the current local time formatted as an 8-character string `"HH:MM:SS"`. The numeric form `TIME` returns elapsed seconds since midnight or total process execution time.

### Syntax Signatures:
```basic
current_time$ = TIME$
secs# = TIME
TIME$ = "HH:MM:SS" : REM Set system time
```

### Operational Rules:
- **`TIME$` String Format**: Always formatted as `"HH:MM:SS"` in 24-hour time.
- **`TIME` Numeric**: Returns double-precision floating-point seconds.
- **Assignment**: Setting `TIME$ = "HH:MM:SS"` updates the system time if permitted.

---

## 2. Code Examples

```basic
10 PRINT "Current Time: "; TIME$
20 PRINT "Seconds since midnight: "; TIME
```
