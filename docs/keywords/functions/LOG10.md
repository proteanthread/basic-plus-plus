# `LOG10` Common Logarithm (Base-10) Function

## 1. BASIC Usage and Function Definition

The `LOG10` function calculates the base-10 logarithm of a positive numeric argument.

### Syntax Signatures:
```basic
result# = LOG10(numeric_expression)
```

### Operational Rules:
- **Domain Constraint**: Requires $x > 0$. Non-positive values trigger Error 5 (`ERR_ILLEGAL_FUNCTION_CALL`).

---

## 2. Code Examples

```basic
10 PRINT LOG10(10000) : REM Outputs 4
```
