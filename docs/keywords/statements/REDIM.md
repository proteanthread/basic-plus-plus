# `REDIM` Dynamic Array Redimensioning Statement

## 1. BASIC Usage and Keyword Definition

The `REDIM` statement dynamically changes the size, dimensions, or bounds of an existing dynamic array during program execution. By default, `REDIM` reallocates the array memory and clears existing contents. When the `PRESERVE` modifier is used, `REDIM` resizes the last dimension of the array while preserving existing element values.

### Syntax Signatures:
```basic
REDIM [PRESERVE] arrayname(subscripts) [AS type] [, arrayname2(...)]
```

### Operational Rules:
- **Dynamic Arrays Only**: Only dynamically allocated arrays (declared with `REDIM` or `$DYNAMIC`) can be resized. Static arrays trigger Error 10 (`ERR_DUPLICATE_DEFINITION`).
- **PRESERVE Option**: Retains existing data within common index boundaries. If dimensions change with `PRESERVE`, only the upper bound of the last dimension may be modified.
- **Memory Allocation**: Backing buffers are safely reallocated or replaced via `realloc` / `calloc` in `engine/src/runtime/arrays.c`.
- **String Management**: Any elements truncated during downward resizing have their strings dereferenced via `str_release()`.

---

## 2. Language Dialect & Compatibility

| Dialect | Syntax | PRESERVE Support | Notes |
|---|---|---|---|
| **GW-BASIC / BASICA** | *None* | No | Required `ERASE` then `DIM` |
| **QuickBASIC / QBASIC** | `REDIM A(100)` | No (`$DYNAMIC` required) | Cleared data on redim |
| **Visual Basic** | `ReDim [Preserve] A(N)` | Yes | Preserves data |
| **BASIC++ (Master)** | `REDIM [PRESERVE] A(N)` | Yes | Native C17 dynamic array manager |

---

## 3. Examples

### Dynamically Growing a Data Buffer with PRESERVE
```basic
10 REDIM LogList$(10)
20 FOR I = 1 TO 10
30   LogList$(I) = "Entry " + NUM$(I)
40 NEXT I
50 REM Dynamically expand buffer to 20 items without losing 1..10
60 REDIM PRESERVE LogList$(20)
70 PRINT "Retained entry 5: "; LogList$(5)
80 PRINT "New array upper bound: "; UBOUND(LogList$)
```
