# `TANH` Hyperbolic Tangent Function

## 1. BASIC Usage and Function Definition

The `TANH` function computes the hyperbolic tangent of a numeric argument:

$$\tanh(x) = \frac{\sinh(x)}{\cosh(x)} = \frac{e^x - e^{-x}}{e^x + e^{-x}}$$

### Syntax Signatures:
```basic
result# = TANH(numeric_expression)
```

### Operational Rules:
- Returns real float in range $(-1.0, +1.0)$.

---

## 2. Code Examples

```basic
10 PRINT TANH(0) : REM Outputs 0
```
