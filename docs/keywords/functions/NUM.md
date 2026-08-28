# `NUM` String to Number Parsing Function

## 1. BASIC Usage and Function Definition

The `NUM` (or `VAL`) function parses a numeric string and returns its double-precision floating-point value.

### Syntax Signatures:
```basic
numeric_val# = NUM(string_expression$)
```

### Operational Rules:
- Reads leading numeric digits, decimal points, and scientific notation exponents (`1.23E4`).
- Returns 0 if string contains no leading numeric characters.

---

## 2. Code Examples

```basic
10 S$ = "123.45"
20 PRINT NUM(S$) * 2 : REM Outputs 246.9
```
