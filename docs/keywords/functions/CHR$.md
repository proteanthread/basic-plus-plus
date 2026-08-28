# `CHR$` Character from ASCII Code Function

## 1. BASIC Usage and Function Definition

The `CHR$` function converts an integer ASCII / byte value ($0$ to $255$) into a 1-character string.

### Syntax Signatures:
```basic
char_str$ = CHR$(code_point%)
```

### Operational Rules:
- Bounded to $0 \le \text{code} \le 255$. Out-of-bounds codes trigger Error 5 (`ERR_ILLEGAL_FUNCTION_CALL`).
- Returns a reference-counted 1-byte string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 PRINT CHR$(65) : REM Outputs "A"
20 PRINT CHR$(34) : REM Outputs double quote (")
```
