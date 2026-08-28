# `CLOG` Complex Natural Logarithm Function

## 1. BASIC Usage and Function Definition

The `CLOG` function computes the principal natural logarithm $\ln(z)$ of a complex number $z$:

$$\ln(z) = \ln|z| + i \arg(z)$$

### Syntax Signatures:
```basic
res_complex = CLOG(complex_val)
```

### Operational Rules:
- Returns principal branch value with imaginary part in $(-\pi, +\pi]$.
- Passing $z = 0$ triggers Error 5 (`ERR_ILLEGAL_FUNCTION_CALL`).

---

## 2. Code Examples

```basic
10 Z = COMPLEX(1, 0)
20 PRINT "CLOG(1) = "; REAL(CLOG(Z)); " + "; IMAG(CLOG(Z)); "i"
```
