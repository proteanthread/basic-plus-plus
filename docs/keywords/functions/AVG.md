# `AVG` Arithmetic Mean / Average Function

## 1. BASIC Usage and Function Definition

The `AVG` function computes the arithmetic mean (average) of two or more numeric arguments or array slices.

### Syntax Signatures:
```basic
mean = AVG(val1, val2 [, val3, ...])
mean = AVG(array())
```

### Operational Rules:
- Computes $\frac{1}{N}\sum_{i=1}^{N} x_i$.

---

## 2. Code Examples

```basic
10 PRINT "Average of 10, 20, 30 = "; AVG(10, 20, 30) : REM Outputs 20
```
