# `JIFFIES` Clock Ticks (60Hz) Function

## 1. BASIC Usage and Function Definition

The `JIFFIES` function returns the number of 1/60th second time intervals (jiffies) elapsed since system startup, matching Commodore 64 and Apple II hardware timer resolution.

### Syntax Signatures:
```basic
jiffies& = JIFFIES
jiffies& = JIFFIES()
```

### Operational Rules:
- Increments at 60Hz (60 ticks per second).

---

## 2. Code Examples

```basic
10 START_J = JIFFIES
20 FOR I = 1 TO 10000 : NEXT I
30 ELAPSED = (JIFFIES - START_J) / 60.0
40 PRINT "Elapsed time: "; ELAPSED; " seconds"
```
