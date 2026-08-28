# `TI` System Timer Ticks Function

## 1. BASIC Usage and Function Definition

The `TI` function returns the 60Hz tick count (jiffies) since system power-on, identical to the Commodore 64 `TI` variable.

### Syntax Signatures:
```basic
t& = TI
t& = TI()
```

### Operational Rules:
- Equivalent to `JIFFIES`.

---

## 2. Code Examples

```basic
10 T1 = TI
20 FOR I = 1 TO 50000 : NEXT I
30 PRINT "Ticks elapsed: "; TI - T1
```
