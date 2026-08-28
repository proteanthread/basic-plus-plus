# `ROUND` Numeric Rounding Function

## 1. BASIC Usage and Function Definition

The `ROUND` function rounds a numeric expression to a designated number of decimal places using standard half-up rounding arithmetic.

### Syntax Signatures:
```basic
rounded# = ROUND(numeric_expression [, decimal_places%])
```

### Operational Rules:
- If `decimal_places%` is omitted, rounds to nearest whole integer.

---

## 2. Code Examples

```basic
10 PRINT ROUND(3.14159, 2) : REM Outputs 3.14
20 PRINT ROUND(4.5)        : REM Outputs 5
```
