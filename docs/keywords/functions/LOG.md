# `LOG` Natural Logarithm (Base-e) Function

## 1. BASIC Usage and Function Definition

The `LOG` function computes the natural logarithm $\ln(x)$ of a positive numeric argument.

### Syntax Signatures:
```basic
result# = LOG(numeric_expression)
```

### Operational Rules:
- **Domain Constraint**: Requires $x > 0$. Arguments $\le 0$ trigger Error 5 (`ERR_ILLEGAL_FUNCTION_CALL`).

---

## 2. Code Examples

```basic
10 PRINT LOG(2.718281828459045) : REM Outputs 1
20 PRINT LOG(1)                 : REM Outputs 0
```
