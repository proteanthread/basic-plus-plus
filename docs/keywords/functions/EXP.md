# `EXP` Exponential Base-e Function

## 1. BASIC Usage and Function Definition

The `EXP` function calculates the mathematical constant $e$ raised to the power of the numeric argument ($e^x$).

### Syntax Signatures:
```basic
result# = EXP(numeric_expression)
```

### Operational Rules:
- Returns real float $> 0$.
- Large positive arguments exceeding float capacity trigger Error 6 (`ERR_OVERFLOW`).

---

## 2. Code Examples

```basic
10 PRINT EXP(0) : REM Outputs 1
20 PRINT EXP(1) : REM Outputs 2.718281828459045
```
