# `MAT_TRN` Matrix Transpose Statement

## 1. BASIC Usage and Keyword Definition

The `MAT ... = TRN(A)` statement computes the matrix transpose $A^T$ of matrix $A$ by swapping row and column indices ($B_{j,i} = A_{i,j}$).

### Syntax Signatures:
```basic
MAT B = TRN(A)
```

### Operational Rules:
- If $A$ is $M\times N$, $B$ must be dimensioned $N\times M$.

---

## 2. Code Examples

```basic
10 DIM A(2, 3), B(3, 2)
20 MAT B = TRN(A)
30 PRINT "Matrix transposed."
```
