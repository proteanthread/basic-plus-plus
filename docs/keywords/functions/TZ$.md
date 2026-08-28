# `TZ$` Timezone Abbreviation String Function

## 1. BASIC Usage and Function Definition

The `TZ$` function returns the short abbreviation string for the current local timezone (e.g. `"UTC"`, `"EST"`, `"MDT"`, `"CST"`).

### Syntax Signatures:
```basic
zone$ = TZ$
zone$ = TZ$()
```

### Operational Rules:
- Equivalent to `TIMEZONE$`.

---

## 2. Code Examples

```basic
10 PRINT "Timezone: "; TZ$
```
