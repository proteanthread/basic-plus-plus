# `IMAG` Imaginary Component Extractor Function

## 1. BASIC Usage and Function Definition

The `IMAG` function extracts the imaginary component $y$ from a complex number $z = x + iy$.

### Syntax Signatures:
```basic
imag_part# = IMAG(complex_val)
```

### Operational Rules:
- Returns real double-precision floating-point value.

---

## 2. Code Examples

```basic
10 Z = COMPLEX(3, -4.5)
20 PRINT "Imaginary part: "; IMAG(Z) : REM Outputs -4.5
```
