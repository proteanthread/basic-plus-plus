# `MAT_INV` Matrix Inversion Function & Statement

## 1. BASIC Usage and Keyword Definition

The `MAT ... = INV(A)` statement computes the matrix inverse $A^{-1}$ of a non-singular square matrix $A$, optionally setting the determinant variable.

### Syntax Signatures:
```basic
MAT B = INV(A)
det# = DET
```

### Operational Rules:
- Calculates inverse using Gaussian elimination with partial pivoting.
- If matrix is singular ($\det A = 0$), triggers Error 11 (`ERR_DIVISION_BY_ZERO`) or sets determinant to 0.

---

## 2. Code Examples

```basic
10 DIM A(2, 2), B(2, 2)
20 A(1, 1) = 4 : A(1, 2) = 7 : A(2, 1) = 2 : A(2, 2) = 6
30 MAT B = INV(A)
40 PRINT "Matrix inverted. Determinant = "; DET
```
