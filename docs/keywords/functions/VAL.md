# `VAL` String Numeric Value Parser Function

## 1. BASIC Usage and Function Definition

The `VAL` function parses a string representation of a number and returns its 64-bit floating-point numeric value.

### Syntax Signatures:
```basic
numeric_val# = VAL(string_expression$)
```

### Operational Rules:
- Skips leading whitespace and parses signs, decimal digits, decimal points, and scientific notation exponents (`1.2E-3`).
- Stops parsing at the first non-numeric character.
- Returns `0` if the string contains no parseable numeric prefix.

---

## 2. Code Examples

```basic
10 S$ = "123.456 meters"
20 PRINT VAL(S$) : REM Outputs 123.456
```
