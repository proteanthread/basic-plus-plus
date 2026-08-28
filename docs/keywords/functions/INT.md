# `INT` Greatest Integer (Floor) Function

## 1. BASIC Usage and Function Definition

The `INT` function returns the largest integer less than or equal to a numeric expression (floor operation).

### Syntax Signatures:
```basic
res = INT(numeric_expression)
```

### Operational Rules:
- Rounds down towards $-\infty$: `INT(4.8) = 4`, `INT(-4.8) = -5`.
- Contrast with `FIX()`, which truncates toward zero (`FIX(-4.8) = -4`).

---

## 2. Code Examples

```basic
10 PRINT INT(3.9)  : REM Outputs 3
20 PRINT INT(-3.1) : REM Outputs -4
```
