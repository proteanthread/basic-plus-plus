# `LCASE$` Lowercase String Conversion Function

## 1. BASIC Usage and Function Definition

The `LCASE$` function converts all uppercase alphabetic characters in a string to lowercase.

### Syntax Signatures:
```basic
lower_str$ = LCASE$(string_expression$)
```

### Operational Rules:
- Converts ASCII `'A'-'Z'` to `'a'-'z'`.
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 S$ = "BASIC++ Master Edition"
20 PRINT LCASE$(S$) : REM Outputs "basic++ master edition"
```
