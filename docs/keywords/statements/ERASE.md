# `ERASE` Array Deallocation and Reinitialization Statement

## 1. BASIC Usage and Keyword Definition

The `ERASE` statement eliminates one or more arrays from active memory or clears their elements back to initial default values. For dynamic arrays, `ERASE` deallocates all memory associated with the array, allowing it to be redimensioned with different bounds using `DIM` or `REDIM`. For static arrays, `ERASE` zeroes out numeric elements and resets string elements to empty strings (`""`) without changing array bounds.

### Syntax Signatures:
```basic
ERASE arrayname [, arrayname2, ...]
```

### Operational Rules:
- **String Memory Cleanup**: All reference-counted strings contained within string arrays are released via `str_release()` to prevent memory leaks.
- **Dynamic Deallocation**: Frees the backing `elements` buffer and unregisters the array from the `ArrayContext`.
- **Static Arrays**: Clears memory in-place with `memset` for numeric types or releases strings for string arrays.
- **Multiple Targets**: Multiple arrays can be specified in a single `ERASE` statement separated by commas.
- **Error Trapping**: Attempting to erase an undeclared array triggers Error 9 (`ERR_SUBSCRIPT_OUT_OF_RANGE`).

---

## 2. Language Dialect & Compatibility

| Dialect | Syntax | Dynamic Action | Static Action |
|---|---|---|---|
| **GW-BASIC / BASICA** | `ERASE A, B$` | Deallocates array memory | Deallocates array memory |
| **QuickBASIC / QBASIC** | `ERASE A, B$` | Frees heap allocation | Reinitializes to zero / empty |
| **Visual Basic** | `Erase A, B` | Clears/deallocates | Reinitializes elements |
| **BASIC++ (Master)** | `ERASE A, B$` | Full memory reclamation & string dereferencing | Reinitializes or deallocates |

---

## 3. Examples

### Reallocating Dynamic Arrays with Different Sizes
```basic
10 DIM DataBuffer(100)
20 PRINT "Allocated initial 100 elements."
30 ERASE DataBuffer
40 DIM DataBuffer(1000)
50 PRINT "Reallocated 1000 elements successfully."
```

### Clearing Multiple Large Arrays
```basic
100 DIM A(500), B$(500), C(50, 50)
110 ERASE A, B$, C
120 PRINT "All temporary memory reclaimed."
```
