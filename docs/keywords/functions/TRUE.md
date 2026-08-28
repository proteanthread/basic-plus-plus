# `TRUE` Boolean True Constant

## 1. BASIC Usage and Keyword Definition

The `TRUE` keyword is a built-in constant evaluating to `-1` (or boolean truth in logical contexts), matching historical Microsoft BASIC two's-complement bitwise truth conventions where `NOT 0 = -1`.

### Syntax Signatures:
```basic
flag% = TRUE
```

### Operational Rules:
- Evaluates to integer `-1` (`0xFFFFFFFFFFFFFFFF` in 64-bit integer logic).
- `NOT TRUE` evaluates to `FALSE` (`0`).

---

## 2. Code Examples

```basic
10 LET ACTIVE% = TRUE
20 IF ACTIVE% THEN PRINT "Condition is TRUE"
```
