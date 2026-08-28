# `READ` Read Data from DATA Statements Statement

## 1. BASIC Usage and Keyword Definition

Reads consecutive constant values from program DATA statements into variables.

### Syntax Signatures:
```basic
READ variable [, variable...]
```

### Error Handling & Boundary Conditions:
- **Error 4 (ERR_OUT_OF_DATA)**: Attempted to READ past last DATA item.
- **Error 13 (ERR_TYPE_MISMATCH)**: String data read into numeric variable.

### Operational Notes:
- Advances internal data pointer automatically.

---

## 2. Code Examples

```basic
10 READ X, Y, Label$
20 PRINT Label$; " at ("; X; ","; Y; ")"
30 DATA 100, 200, "Target"
```
