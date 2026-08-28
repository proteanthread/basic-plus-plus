# `UTC` Unix Timestamp Function

## 1. BASIC Usage and Function Definition

The `UTC` function (and alias `CLOCK`) returns the current universal Coordinated Universal Time (UTC) Unix epoch timestamp in seconds.

### Syntax Signatures:
```basic
timestamp& = UTC
timestamp& = UTC()
```

### Operational Rules:
- Returns integer/float seconds since January 1, 1970 00:00:00 UTC.

---

## 2. Code Examples

```basic
10 PRINT "Current Unix Epoch Timestamp: "; UTC
```
