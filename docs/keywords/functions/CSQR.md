# `CSQR` Complex Square Root Function

## 1. BASIC Usage and Function Definition

The `CSQR` function computes the principal complex square root $\sqrt{z}$ of a complex number $z = x + iy$.

### Syntax Signatures:
```basic
res_complex = CSQR(complex_val)
```

### Operational Rules:
- Returns square root with non-negative real part ($\text{Re}(\sqrt{z}) \ge 0$).

---

## 2. Code Examples

```basic
10 Z = COMPLEX(-1, 0)
20 RES = CSQR(Z)
30 PRINT "sqrt(-1) = "; REAL(RES); " + "; IMAG(RES); "i" : REM Outputs 0 + 1i
```
