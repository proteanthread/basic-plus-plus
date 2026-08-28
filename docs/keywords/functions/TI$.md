# `TI$` Timer Clock String Function & Variable

## 1. BASIC Usage and Keyword Definition

The `TI$` variable/function provides Commodore 64 compatible 6-digit string time representation formatted as `"HHMMSS"`. In statement form, `TI$ = "HHMMSS"` resets the timer counter.

### Syntax Signatures:
```basic
time_str$ = TI$
TI$ = "120000" : REM Reset timer to 12:00:00
```

### Operational Rules:
- Returns a 6-character string `"HHMMSS"` representing hours, minutes, and seconds.

---

## 2. Code Examples

```basic
10 PRINT "TI$ Time: "; TI$
```
