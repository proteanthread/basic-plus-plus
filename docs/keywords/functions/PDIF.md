# `PDIF` Positive Difference Function

## 1. BASIC Usage and Function Definition

The `PDIF` function calculates the positive difference between two numeric arguments, returning $x - y$ if $x > y$, and $0$ otherwise.

### Syntax Signatures:
```basic
res = PDIF(x, y)
```

### Operational Rules:
- Computes $\max(0, x - y)$.

---

## 2. Code Examples

```basic
10 PRINT PDIF(10, 4) : REM Outputs 6
20 PRINT PDIF(4, 10) : REM Outputs 0
```
