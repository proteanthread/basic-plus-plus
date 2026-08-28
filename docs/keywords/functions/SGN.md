# `SGN` Signum Function

## 1. BASIC Usage and Function Definition

The `SGN` function indicates the mathematical sign of a numeric expression, returning $-1$ for negative numbers, $0$ for zero, and $+1$ for positive numbers.

### Syntax Signatures:
```basic
sign_val% = SGN(numeric_expression)
```

### Operational Rules:
- Returns $-1$ if $x < 0$.
- Returns $0$ if $x = 0$.
- Returns $+1$ if $x > 0$.

---

## 2. Code Examples

```basic
10 PRINT SGN(-42) : REM Outputs -1
20 PRINT SGN(0)   : REM Outputs 0
30 PRINT SGN(99)  : REM Outputs 1
```
