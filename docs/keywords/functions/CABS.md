# `CABS` Complex Absolute Value / Magnitude Function

## 1. BASIC Usage and Function Definition

The `CABS` function computes the absolute value (magnitude / modulus) of a complex number $z = x + iy$:

$$|z| = \sqrt{x^2 + y^2}$$

### Syntax Signatures:
```basic
magnitude# = CABS(complex_val)
magnitude# = CABS(real_val, imag_val)
```

### Operational Rules:
- Returns non-negative Euclidean length of the complex vector.

---

## 2. Code Examples

```basic
10 Z = COMPLEX(3, 4)
20 PRINT "Magnitude |3 + 4i| = "; CABS(Z) : REM Outputs 5
```
