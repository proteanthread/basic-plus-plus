# `TZ` Timezone Offset Function

## 1. BASIC Usage and Function Definition

The `TZ` function returns the current local timezone offset in hours relative to UTC (e.g. `-5` for EST, `-7` for MDT, `0` for UTC).

### Syntax Signatures:
```basic
offset_hours = TZ
offset_hours = TZ()
```

### Operational Rules:
- Returns numeric floating-point or integer offset in hours.

---

## 2. Code Examples

```basic
10 PRINT "Timezone Offset from UTC: "; TZ; " hours"
```
