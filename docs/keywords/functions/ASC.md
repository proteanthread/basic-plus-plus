# `ASC` ASCII / Character Byte Value Function

## 1. BASIC Usage and Function Definition

The `ASC` function returns the numeric ASCII / byte value ($0$ to $255$) of the first character of a string expression.

### Syntax Signatures:
```basic
code% = ASC(string_expression$)
```

### Operational Rules:
- Returns integer in range $0 \le \text{code} \le 255$.
- Passing an empty string (`""`) triggers Error 5 (`ERR_ILLEGAL_FUNCTION_CALL`).

---

## 2. Code Examples

```basic
10 PRINT ASC("A") : REM Outputs 65
20 PRINT ASC("0") : REM Outputs 48
```
