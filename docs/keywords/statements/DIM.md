# `DIM` Array Dimension Declaration Statement

## 1. BASIC Usage and Keyword Definition

The `DIM` statement allocates and dimensions one or more subscripted array variables in memory. `DIM` sets the upper boundary (and optionally lower boundary via `TO` or `OPTION BASE`) for each dimension, allocating contiguous memory structures for numeric (integer, single, double) or string array elements.

### Syntax Signatures:
```basic
DIM [SHARED] varname(subscript1 [, subscript2, ...]) [AS type] [, varname2(...)]
DIM varname(lower TO upper [, lower2 TO upper2, ...])
```

### Operational Rules:
- **Default Lower Bound**: Set by `OPTION BASE 0` (default: index 0) or `OPTION BASE 1` (index 1).
- **Explicit Bounds (`TO`)**: Allows explicit lower and upper indices (e.g. `DIM Grid(-10 TO 10, 0 TO 100)`).
- **Maximum Dimensions**: BASIC++ supports up to 8 dimensions (`BPP_ARRAY_MAX_DIMS = 8`).
- **Default Dimensioning**: If an array is referenced without an explicit `DIM`, it is automatically allocated with an upper bound of 10 for each referenced subscript.
- **Initial Values**: Numeric arrays are zero-initialized (`0` / `0.0`); string arrays are initialized to empty strings (`""`).
- **Dynamic vs Static**: Arrays can be deallocated and reallocated using `ERASE` or resized dynamically with `REDIM`.

---

## 2. Language Dialect & Compatibility

| Dialect | Syntax | Max Dims | Base Index | Notes |
|---|---|---|---|---|
| **GW-BASIC / BASICA** | `DIM A(10, 20)` | Up to 255 (RAM limited) | 0 (or 1 via `OPTION BASE 1`) | Static arrays only |
| **QuickBASIC / QBASIC** | `DIM A(1 TO 100) AS INTEGER` | Up to 60 | Configurable via `TO` | Supports `AS type`, `SHARED` |
| **ECMA-116 Full BASIC** | `DIM A(10, 20)` | Multi-dimensional | 1 default | Standard matrix support |
| **BASIC++ (Master)** | `DIM [SHARED] A(...) [AS type]` | 8 dimensions | Configurable (0, 1, or `TO`) | Zero-initialized, 640MB memory pool |

---

## 3. Lexical, AST, and VM Processing

1. **AST Node**: `NODE_STMT_DIM` parsed in `engine/src/statements/dim.c`.
2. **Runtime Memory**: Handled by `ArrayContext` in `engine/src/runtime/arrays.c`.
3. **Data Structure (`BppArray`)**:
   ```c
   typedef struct {
       char name[64];
       BValueType elem_type;
       int num_dims;
       int lower_bounds[8];
       int upper_bounds[8];
       size_t total_elements;
       BValue *elements;
   } BppArray;
   ```
4. **Subscript Index Calculation**:
   $$\text{offset} = \sum_{k=1}^{D} \left( (i_k - L_k) \times \prod_{m=k+1}^{D} (U_m - L_m + 1) \right)$$

---

## 4. Examples

### Multidimensional Lookup Table
```basic
10 OPTION BASE 1
20 DIM Matrix(10, 10) AS DOUBLE
30 FOR R = 1 TO 10
40   FOR C = 1 TO 10
50     Matrix(R, C) = R * C
60   NEXT C
70 NEXT R
80 PRINT "Matrix(5, 7) = "; Matrix(5, 7)
```

### Explicit Range Offsets with Negative Indices
```basic
100 DIM Temperature(-40 TO 100)
110 FOR T = -40 TO 100
120   Temperature(T) = (T * 9 / 5) + 32
130 NEXT T
140 PRINT "0 C in Fahrenheit: "; Temperature(0)
```
