# `SQR` Square Root Function

## 1. BASIC Usage and Function Definition

The `SQR` function computes the non-negative square root $\sqrt{x}$ of a numeric expression.

### Syntax Signatures:
```basic
root# = SQR(numeric_expression)
```

### Operational Rules:
- **Domain Constraint**: Requires $x \ge 0$. Negative arguments trigger Error 5 (`ERR_ILLEGAL_FUNCTION_CALL`) in real mode (for complex square roots, use `CSQR`).

---

## 2. Code Examples

```basic
10 PRINT SQR(16) : REM Outputs 4
20 PRINT SQR(2)  : REM Outputs 1.414213562373095
```
