# `MAT_IDN` Matrix Identity Initialization Statement

## 1. BASIC Usage and Keyword Definition

The `MAT ... = IDN` statement initializes a square matrix into an identity matrix with ones along the principal diagonal and zeros elsewhere.

### Syntax Signatures:
```basic
MAT A = IDN
MAT A = IDN(rows%, cols%)
```

### Operational Rules:
- Sets $A_{i,j} = 1$ when $i = j$, and $0$ when $i \ne j$.
- Requires square dimensions ($N\times N$).

---

## 2. Code Examples

```basic
10 DIM I(3, 3)
20 MAT I = IDN
30 PRINT "3x3 Identity matrix created."
```
