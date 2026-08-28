# `CARG` Complex Argument / Phase Angle Function

## 1. BASIC Usage and Function Definition

The `CARG` function computes the principal argument (phase angle $\theta$) of a complex number $z = x + iy$ in radians:

$$\theta = \text{atan2}(y, x)$$

### Syntax Signatures:
```basic
phase# = CARG(complex_val)
phase# = CARG(real_val, imag_val)
```

### Operational Rules:
- Returns radians in range $(-\pi, +\pi]$.

---

## 2. Code Examples

```basic
10 Z = COMPLEX(0, 1)
20 PRINT "Phase angle of i = "; CARG(Z); " (expected PI/2)"
```
