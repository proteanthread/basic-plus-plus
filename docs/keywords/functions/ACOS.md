# `ACOS` Arccosine Trigonometric Function

## 1. BASIC Usage and Function Definition

The `ACOS` function computes the principal arccosine (inverse cosine) of a numeric argument in radians.

### Syntax Signatures:
```basic
radians# = ACOS(numeric_expression)
```

### Operational Rules:
- **Domain Constraint**: Requires $-1.0 \le x \le 1.0$. Out-of-domain arguments trigger Error 5 (`ERR_ILLEGAL_FUNCTION_CALL`).
- **Return Range**: Returns radians in range $[0, \pi]$.

---

## 2. Code Examples

```basic
10 PRINT "ACOS(0.5) in radians = "; ACOS(0.5)
```
