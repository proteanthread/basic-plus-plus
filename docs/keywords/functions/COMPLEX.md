# `COMPLEX` Complex Number Constructor Function

## 1. BASIC Usage and Function Definition

The `COMPLEX` function constructs a complex number structure from real and imaginary floating-point components.

### Syntax Signatures:
```basic
z = COMPLEX(real_part, imag_part)
```

### Operational Rules:
- Returns a complex number representing $z = x + iy$.

---

## 2. Code Examples

```basic
10 Z = COMPLEX(3, -4)
20 PRINT "Real: "; REAL(Z); " Imag: "; IMAG(Z)
```
