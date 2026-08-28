# `FIX` Integer Truncation Function

## 1. BASIC Usage and Function Definition

The `FIX` function returns the truncated integer part of a numeric expression by stripping fractional decimals towards zero.

### Syntax Signatures:
```basic
res = FIX(numeric_expression)
```

### Operational Rules:
- Truncates towards zero: `FIX(3.7) = 3`, `FIX(-3.7) = -3`.
- Contrast with `INT()`, which floors towards $-\infty$ (`INT(-3.7) = -4`).

---

## 2. Code Examples

```basic
10 PRINT FIX(4.9)  : REM Outputs 4
20 PRINT FIX(-4.9) : REM Outputs -4
```
