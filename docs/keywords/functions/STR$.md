# `STR$` String Representation of Number Function

## 1. BASIC Usage and Function Definition

The `STR$` function converts a numeric expression into a formatted string, prepending a leading space for positive numbers and a minus sign for negative numbers.

### Syntax Signatures:
```basic
res$ = STR$(numeric_expression)
```

### Operational Rules:
- Positive numbers include a leading space; negative numbers start with `"-"`.
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 PRINT "["; STR$(42); "]"  : REM Outputs "[ 42]"
20 PRINT "["; STR$(-42); "]" : REM Outputs "[-42]"
```
