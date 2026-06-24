# Advanced Matrix Functions in BASIC++

**Version 4.1.1**


---

## Table of Contents

- Declaring Arrays
- Array Access
- ERASE
- MAT Operations (Dartmouth Style)
  - MAT READ
  - MAT PRINT
  - MAT Assignment
  - MAT Special Matrices
- Matrix Arithmetic
- Special Matrices
- Transpose and Inverse
- Practical Examples
  - Grade Averages
  - Solving Linear Equations (Ax = b)
- Array Functions
- Limitations

---

BASIC++ supports `DIM` arrays and classic Dartmouth BASIC `MAT` (matrix) operations for working with tables of data.

---

## 1. Declaring Arrays

```basic
DIM A(10)                ' 1D array, 11 elements (0-10)
DIM A(5, 5)              ' 2D array (6x6 matrix)
DIM B(100)               ' Large 1D array
DIM C$(20)               ' String array

OPTION BASE 0            ' Arrays start at index 0 (default)
OPTION BASE 1            ' Arrays start at index 1
```

- Maximum dimensions: **2** (rows × columns)
- Maximum arrays: **256** per program

---

## 2. Array Access

```basic
DIM SCORES(10)
SCORES(1) = 95
SCORES(2) = 87
PRINT SCORES(1) + SCORES(2)     ' 182

DIM GRID(3, 3)
GRID(1, 1) = 5
GRID(2, 3) = 10
PRINT GRID(1, 1) * GRID(2, 3)   ' 50
```

---

## 3. ERASE

```basic
ERASE A                 ' Delete array A, free memory
ERASE A, B, C           ' Delete multiple arrays
```

---

## 4. MAT Operations (Dartmouth Style)

These classic matrix operations work on 2D arrays. They follow the original Dartmouth BASIC MAT specification.

### MAT READ

Read values from `DATA` into a matrix:

```basic
10 DIM A(3, 3)
20 MAT READ A
30 DATA 1, 2, 3
40 DATA 4, 5, 6
50 DATA 7, 8, 9
```

### MAT PRINT

Print a matrix in formatted rows:

```basic
10 MAT PRINT A;          ' semicolon = compact
20 MAT PRINT A,           ' comma = tabbed columns
```

Output (with semicolon):
```
 1  2  3
 4  5  6
 7  8  9
```

### MAT Assignment

| Operation | Description |
|-----------|-------------|
| `MAT B = A` | Copy matrix A to B |
| `MAT C = A + B` | Element-wise addition |
| `MAT C = A - B` | Element-wise subtraction |
| `MAT C = A * B` | Matrix multiplication |
| `MAT C = (K) * A` | Scalar multiplication |

### MAT Special Matrices

| Operation | Description |
|-----------|-------------|
| `MAT A = ZER` | Fill with zeros |
| `MAT A = CON` | Fill with ones |
| `MAT A = IDN` | Identity matrix (diagonal = 1) |
| `MAT A = TRN(B)` | Transpose of B |
| `MAT A = INV(B)` | Inverse of B |

---

## 5. Matrix Arithmetic

**Addition and Subtraction** — Both matrices must have the same dimensions:

```basic
DIM A(3,3), B(3,3), C(3,3)
MAT READ A
MAT READ B
MAT C = A + B
MAT PRINT C
```

**Matrix Multiplication** — `A(m,n) * B(n,p) = C(m,p)`, inner dimensions must match:

```basic
DIM A(2,3), B(3,2), C(2,2)
MAT READ A
MAT READ B
MAT C = A * B
MAT PRINT C
```

**Scalar Multiplication** — The scalar must be in parentheses:

```basic
K = 2.5
MAT B = (K) * A
```

---

## 6. Special Matrices

**Zero Matrix (ZER)** — All elements set to 0:

```basic
DIM A(3,3)
MAT A = ZER
' A = [[0,0,0],[0,0,0],[0,0,0]]
```

**Constant Matrix (CON)** — All elements set to 1:

```basic
MAT A = CON
' A = [[1,1,1],[1,1,1],[1,1,1]]
```

**Identity Matrix (IDN)** — Diagonal = 1, all others = 0 (must be square):

```basic
MAT A = IDN
' A = [[1,0,0],[0,1,0],[0,0,1]]
```

---

## 7. Transpose and Inverse

**Transpose** — Swap rows and columns:

```basic
DIM A(2,3), B(3,2)
MAT READ A
MAT B = TRN(A)
MAT PRINT B
```

**Inverse** — Compute the matrix inverse (square matrices only):

```basic
DIM A(3,3), B(3,3)
MAT READ A
MAT B = INV(A)
MAT PRINT B
```

The inverse exists only if the determinant is non-zero. If the matrix is singular, an error is raised.

---

## 8. Practical Examples

### A. Grade Averages

```basic
10 DIM GRADES(5, 4)      ' 5 students, 4 tests
20 MAT READ GRADES
30 FOR S = 1 TO 5
40   TOTAL = 0
50   FOR T = 1 TO 4
60     TOTAL = TOTAL + GRADES(S, T)
70   NEXT T
80   PRINT "Student"; S; "avg:"; TOTAL / 4
90 NEXT S
100 DATA 90,85,92,88
110 DATA 78,82,75,80
120 DATA 95,98,97,93
130 DATA 60,65,70,68
140 DATA 88,90,85,87
```

### B. Solving Linear Equations (Ax = b)

```basic
10 DIM A(3,3), B(3,3), X(3,1), C(3,1)
20 REM A * X = C, so X = INV(A) * C
30 MAT READ A
40 MAT READ C
50 MAT B = INV(A)
60 MAT X = B * C
70 PRINT "Solution:"
80 MAT PRINT X
```

---

## 9. Array Functions

| Function | Description |
|----------|-------------|
| `LBOUND(A, dim)` | Lower bound of dimension |
| `UBOUND(A, dim)` | Upper bound of dimension |

```basic
DIM SCORES(1 TO 100)
PRINT LBOUND(SCORES, 1)   ' 1
PRINT UBOUND(SCORES, 1)   ' 100
```

---

## 10. Limitations

- Maximum 2 dimensions per array
- Maximum subscript value: 32767
- Maximum 256 arrays total
- Matrix operations only on numeric arrays
- `INV` limited by floating-point precision
- No sparse matrix support
