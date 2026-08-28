# `LOG2` Binary Logarithm (Base-2) Function

## 1. BASIC Usage and Function Definition

The `LOG2` function calculates the binary (base-2) logarithm of a positive numeric argument.

### Syntax Signatures:
```basic
result# = LOG2(numeric_expression)
```

### Operational Rules:
- **Domain Constraint**: Requires $x > 0$. Arguments $\le 0$ trigger Error 5 (`ERR_ILLEGAL_FUNCTION_CALL`).

---

## 2. Code Examples

```basic
10 PRINT LOG2(256)  : REM Outputs 8
20 PRINT LOG2(1024) : REM Outputs 10
```
