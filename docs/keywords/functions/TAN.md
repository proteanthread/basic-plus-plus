# `TAN` Tangent Trigonometric Function

## 1. BASIC Usage and Function Definition

The `TAN` function computes the trigonometric tangent of an angle expressed in radians.

### Syntax Signatures:
```basic
result# = TAN(angle_radians)
```

### Operational Rules:
- Evaluates $\sin(\theta)/\cos(\theta)$.
- Singularities near $(2k+1)\pi/2$ trigger Error 6 (`ERR_OVERFLOW`).

---

## 2. Code Examples

```basic
10 PRINT TAN(0)                 : REM Outputs 0
20 PRINT TAN(0.785398163397448) : REM Outputs 1 (approx PI/4)
```
