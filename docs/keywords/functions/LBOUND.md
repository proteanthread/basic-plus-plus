# `LBOUND` Lower Bound Array Function

## 1. BASIC Usage and Function Definition

The `LBOUND` function returns the lowest valid subscript index for a designated dimension of an array. If no dimension is specified, `LBOUND` defaults to the first dimension (dimension 1).

### Syntax Signatures:
```basic
result = LBOUND(arrayname [, dimension])
```

### Operational Rules:
- **Default Dimension**: If `dimension` is omitted, dimension 1 is queried.
- **Dimension Range**: `dimension` must be an integer between 1 and the array's total dimension count ($1 \le \text{dim} \le D$).
- **Base Compatibility**: Returns `0` (or `1` under `OPTION BASE 1`), or the custom lower bound declared via `DIM A(lower TO upper)`.
- **Return Type**: `VAL_NUMBER` (integer integer value).

---

## 2. Language Dialect & Compatibility

| Dialect | Syntax | Default Base | Custom Lower Bound | Notes |
|---|---|---|---|---|
| **GW-BASIC / BASICA** | *None* | 0 or 1 | No | Not available in 1980s GW-BASIC |
| **QuickBASIC / QBASIC** | `LBOUND(A, 1)` | 0 or 1 | Yes (`TO`) | Standard QuickBASIC function |
| **Visual Basic** | `LBound(A, 1)` | 0 or 1 | Yes | Standard |
| **BASIC++ (Master)** | `LBOUND(A [, dim])` | 0 or 1 | Yes (`TO`) | Full multidimensional inspection |

---

## 3. Examples

### Generic Array Loop Traverser
```basic
10 DIM Buffer(-15 TO 25)
20 FOR I = LBOUND(Buffer) TO UBOUND(Buffer)
30   Buffer(I) = I * 2
40 NEXT I
50 PRINT "Processed elements from "; LBOUND(Buffer); " to "; UBOUND(Buffer)
```

### Multidimensional Array Boundary Checking
```basic
100 DIM Grid(1 TO 10, -5 TO 5)
110 PRINT "Dim 1 lower bound: "; LBOUND(Grid, 1)
120 PRINT "Dim 2 lower bound: "; LBOUND(Grid, 2)
```
