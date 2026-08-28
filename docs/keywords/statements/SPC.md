# `SPC` Print Spaces Formatting Function

## 1. BASIC Usage and Keyword Definition

Outputs a specified number of consecutive ASCII space characters inside a PRINT or LPRINT statement.

### Syntax Signatures:
```basic
PRINT SPC(space_count%)
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Count < 0 or > 255.

### Operational Notes:
- Valid only inside PRINT/LPRINT/WRITE argument lists.

---

## 2. Code Examples

```basic
10 PRINT "Col1"; SPC(10); "Col2"
```
