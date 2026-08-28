# `PI` Archimedes' Mathematical Constant Function / Constant

## 1. BASIC Usage and Function Definition

The `PI` function (or constant identifier) returns the high-precision mathematical constant $\pi \approx 3.14159265358979323846$.

### Syntax Signatures:
```basic
pi_val# = PI
pi_val# = PI()
```

### Operational Rules:
- Returns standard 64-bit IEEE double-precision representation of $\pi$.

---

## 2. Code Examples

```basic
10 RADIUS = 5
20 AREA = PI * RADIUS ^ 2
30 PRINT "Circle area: "; AREA
```
