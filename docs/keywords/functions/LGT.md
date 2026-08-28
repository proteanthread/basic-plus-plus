# `LGT` Common (Base-10) Logarithm Function

## 1. BASIC Usage and Function Definition

The `LGT` (or `LOG10`) function computes the base-10 logarithm $\log_{10}(x)$ of a positive numeric argument.

### Syntax Signatures:
```basic
result# = LGT(numeric_expression)
```

### Operational Rules:
- **Domain Constraint**: Requires $x > 0$. Arguments $\le 0$ trigger Error 5 (`ERR_ILLEGAL_FUNCTION_CALL`).

---

## 2. Code Examples

```basic
10 PRINT LGT(100)  : REM Outputs 2
20 PRINT LGT(1000) : REM Outputs 3
```
