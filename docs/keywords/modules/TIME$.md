# `TIME$` System Time String Function & Variable

## 1. BASIC Usage and Keyword Definition

The `TIME$` variable/function returns the current local system time formatted as an 8-character string `"HH:MM:SS"`. In statement syntax, `TIME$ = "HH:MM:SS"` updates the system time.

### Syntax Signatures:
```basic
cur_time$ = TIME$
TIME$ = "12:00:00"
```

### Operational Rules:
- Returns an 8-character 24-hour time string `"HH:MM:SS"`.
- Modifying time requires `CAP_SYS` permissions.

---

## 2. Code Examples

```basic
10 PRINT "Current System Time: "; TIME$
```
