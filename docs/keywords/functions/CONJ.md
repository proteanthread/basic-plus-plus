# `CONJ` Complex Conjugate Function

## 1. BASIC Usage and Function Definition

The `CONJ` function computes the complex conjugate of a complex number $z = x + iy$:

$$\bar{z} = x - iy$$

### Syntax Signatures:
```basic
res_complex = CONJ(complex_val)
```

### Operational Rules:
- Negates the sign of the imaginary component.

---

## 2. Code Examples

```basic
10 Z = COMPLEX(3, 4)
20 Z_BAR = CONJ(Z)
30 PRINT "Conjugate: "; REAL(Z_BAR); " + "; IMAG(Z_BAR); "i" : REM Outputs 3 + -4i
```
