# `NUM$` Numeric Formatted String Function

## 1. BASIC Usage and Function Definition

The `NUM$` function converts a numeric value into a cleanly formatted decimal string representation without leading spaces for positive numbers.

### Syntax Signatures:
```basic
num_str$ = NUM$(numeric_expression)
```

### Operational Rules:
- Converts number to string without the leading sign space produced by `STR$()`.
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 PRINT "["; NUM$(42); "]" : REM Outputs "[42]" (no leading space)
```
