# `CEXP` Complex Exponential Function

## 1. BASIC Usage and Function Definition

The `CEXP` function calculates the exponential $e^z$ of a complex number $z = x + iy$:

$$e^{x + iy} = e^x (\cos y + i \sin y)$$

### Syntax Signatures:
```basic
res_complex = CEXP(complex_val)
```

### Operational Rules:
- Returns complex number result structure.

---

## 2. Code Examples

```basic
10 Z = COMPLEX(0, 3.1415926535)
20 RES = CEXP(Z)
30 PRINT "e^(i*PI) = "; REAL(RES); " + "; IMAG(RES); "i (expected -1)"
```
