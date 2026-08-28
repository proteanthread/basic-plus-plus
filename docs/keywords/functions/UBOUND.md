# `UBOUND` Upper Bound Array Function

## 1. BASIC Usage and Function Definition

The `UBOUND` function returns the highest valid subscript index for a designated dimension of an array. If no dimension is specified, `UBOUND` defaults to the first dimension (dimension 1).

### Syntax Signatures:
```basic
result = UBOUND(arrayname [, dimension])
```

### Operational Rules:
- **Default Dimension**: If `dimension` is omitted, dimension 1 is queried.
- **Dimension Parameter**: Must be an integer between 1 and the total number of dimensions in the array ($1 \le \text{dim} \le D$).
- **Return Value**: Integer number representing the upper index limit declared in `DIM` or `REDIM`.
- **Return Type**: `VAL_NUMBER`.

---

## 2. Language Dialect & Compatibility

| Dialect | Syntax | Default Base | Notes |
|---|---|---|---|
| **GW-BASIC / BASICA** | *None* | N/A | Not supported natively in GW-BASIC |
| **QuickBASIC / QBASIC** | `UBOUND(A, 1)` | 1 default | Standard QuickBASIC |
| **Visual Basic** | `UBound(A, 1)` | 1 default | Standard |
| **BASIC++ (Master)** | `UBOUND(A [, dim])` | 1 default | Full multidimensional inspection |

---

## 3. Examples

### Iterating Over Array Bounds
```basic
10 DIM Values(25)
20 FOR I = LBOUND(Values) TO UBOUND(Values)
30   Values(I) = I * 10
40 NEXT I
50 PRINT "Array has "; UBOUND(Values) - LBOUND(Values) + 1; " total elements."
```

### Checking Dimensions of 2D Matrix
```basic
100 DIM Board(8, 8)
110 PRINT "Rows: "; UBOUND(Board, 1)
120 PRINT "Cols: "; UBOUND(Board, 2)
```
