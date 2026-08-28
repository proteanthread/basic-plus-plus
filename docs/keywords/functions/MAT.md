# `MAT` Matrix Arithmetic Operations

## 1. BASIC Usage and Keyword Definition

The `MAT` statement suite executes matrix assignment, scalar multiplication, matrix addition, subtraction, and multiplication.

### Syntax Signatures:
```basic
MAT C = A + B
MAT C = A - B
MAT C = A * B
MAT C = (scalar) * A
```

### Operational Rules:
- Enforces matrix dimension matching for addition/subtraction ($M\times N$) and multiplication ($M\times K$ by $K\times N$).
- Adheres to dynamic `OPTION BASE 0` or `1` active bounds.

---

## 2. Code Examples

```basic
10 DIM A(2, 2), B(2, 2), C(2, 2)
20 MAT C = A + B
30 PRINT "Matrix addition complete."
```
