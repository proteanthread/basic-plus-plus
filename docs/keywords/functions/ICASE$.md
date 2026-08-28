# `ICASE$` Inverted Case String Transformation Function

## 1. BASIC Usage and Function Definition

The `ICASE$` function inverts the letter case of all alphabetic characters in a string, converting uppercase characters to lowercase and lowercase to uppercase.

### Syntax Signatures:
```basic
res$ = ICASE$(string_expression$)
```

### Operational Rules:
- Non-alphabetic characters remain unchanged.
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 PRINT ICASE$("Hello World!") : REM Outputs "hELLO wORLD!"
```
