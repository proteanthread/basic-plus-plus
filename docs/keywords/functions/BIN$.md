# `BIN$` Binary String Conversion Function

## 1. BASIC Usage and Function Definition

The `BIN$` function converts an integer expression into its base-2 binary text representation.

### Syntax Signatures:
```basic
bin_str$ = BIN$(numeric_expression)
```

### Operational Rules:
- Converts integer into a sequence of `'0'` and `'1'` ASCII characters.
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 PRINT BIN$(5)   : REM Outputs "101"
20 PRINT BIN$(255) : REM Outputs "11111111"
```
