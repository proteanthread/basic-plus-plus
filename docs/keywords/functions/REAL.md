# `REAL` Real Component Extractor Function

## 1. BASIC Usage and Function Definition

The `REAL` function extracts the real component $x$ from a complex number $z = x + iy$.

### Syntax Signatures:
```basic
real_part# = REAL(complex_val)
```

### Operational Rules:
- Returns real double-precision floating-point value.

---

## 2. Code Examples

```basic
10 Z = COMPLEX(3.14, 2.71)
20 PRINT "Real component: "; REAL(Z) : REM Outputs 3.14
```
