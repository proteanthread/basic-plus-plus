# `ASIN` Arcsine Trigonometric Function

## 1. BASIC Usage and Function Definition

The `ASIN` function computes the principal arcsine (inverse sine) of a numeric argument in radians.

### Syntax Signatures:
```basic
radians# = ASIN(numeric_expression)
```

### Operational Rules:
- **Domain Constraint**: Requires $-1.0 \le x \le 1.0$. Out-of-domain arguments trigger Error 5 (`ERR_ILLEGAL_FUNCTION_CALL`).
- **Return Range**: Returns radians in range $[-\pi/2, +\pi/2]$.

---

## 2. Code Examples

```basic
10 PRINT "ASIN(1.0) = "; ASIN(1.0); " (expected PI/2)"
```
