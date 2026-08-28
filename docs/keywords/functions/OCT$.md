# `OCT$` Octal String Conversion Function

## 1. BASIC Usage and Function Definition

The `OCT$` function converts an integer expression into its base-8 octal text representation.

### Syntax Signatures:
```basic
oct_str$ = OCT$(numeric_expression)
```

### Operational Rules:
- Converts integer into a sequence of octal digits (`'0'` through `'7'`).
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 PRINT OCT$(8)  : REM Outputs "10"
20 PRINT OCT$(64) : REM Outputs "100"
```
