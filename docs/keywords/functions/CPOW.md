# `CPOW` Complex Power / Exponentiation Function

## 1. BASIC Usage and Function Definition

The `CPOW` function computes the complex power $z^w$ where base $z$ and exponent $w$ are complex numbers:

$$z^w = e^{w \ln z}$$

### Syntax Signatures:
```basic
res_complex = CPOW(complex_base, complex_exp)
```

### Operational Rules:
- Returns complex power evaluated using principal logarithm branch.

---

## 2. Code Examples

```basic
10 Z = COMPLEX(0, 1) : REM i
20 W = COMPLEX(0, 1) : REM i
30 RES = CPOW(Z, W)   : REM i^i = e^(-PI/2) approx 0.20788
40 PRINT "i^i = "; REAL(RES)
```
