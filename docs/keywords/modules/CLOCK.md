# `CLOCK` Numeric Timestamp Function

## 1. BASIC Usage and Function Definition

The `CLOCK` function returns the current system Unix epoch timestamp in seconds as a double-precision numeric value (`VAL_NUMBER`).

### Syntax Signatures:
```basic
ts# = CLOCK
ts# = CLOCK()
```

### Operational Rules:
- Returns seconds elapsed since January 1, 1970 00:00:00 UTC.

---

## 2. Code Examples

```basic
10 START_TS# = CLOCK
20 REM Perform computation
30 PRINT "Start Unix epoch: "; START_TS#
```
