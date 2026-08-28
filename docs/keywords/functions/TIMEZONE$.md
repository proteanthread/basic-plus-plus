# `TIMEZONE$` Timezone String Function

## 1. BASIC Usage and Function Definition

The `TIMEZONE$` function (and alias `TZ$`) returns the current system timezone abbreviation string (e.g. `"UTC"`, `"EST"`, `"MDT"`, `"PST"`).

### Syntax Signatures:
```basic
zone$ = TIMEZONE$
zone$ = TIMEZONE$()
zone$ = TZ$
```

### Operational Rules:
- Returns timezone abbreviation retrieved from `libplatform`.

---

## 2. Code Examples

```basic
10 PRINT "Current Timezone: "; TIMEZONE$
```
