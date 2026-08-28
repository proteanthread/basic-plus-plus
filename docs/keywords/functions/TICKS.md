# `TICKS` High-Resolution Millisecond Timer Function

## 1. BASIC Usage and Function Definition

The `TICKS` function returns a high-resolution monotonic tick count in milliseconds elapsed since VM process startup.

### Syntax Signatures:
```basic
ms& = TICKS
ms& = TICKS()
```

### Operational Rules:
- Returns an integer/float representing elapsed time in milliseconds ($1000\text{ ticks} = 1\text{ second}$).
- Monotonically increasing clock from `libplatform`.

---

## 2. Code Examples

```basic
10 T0 = TICKS
20 FOR I = 1 TO 100000 : NEXT I
30 T1 = TICKS
40 PRINT "Loop execution time: "; (T1 - T0); " ms"
```
