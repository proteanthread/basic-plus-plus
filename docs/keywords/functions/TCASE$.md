# `TCASE$` Title Case String Transformation Function

## 1. BASIC Usage and Function Definition

The `TCASE$` function converts a string into title case by capitalizing the first letter of each word and converting all subsequent letters to lowercase.

### Syntax Signatures:
```basic
res$ = TCASE$(string_expression$)
```

### Operational Rules:
- Capitalizes initial character after whitespace or punctuation delimiters.
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 PRINT TCASE$("war and peace") : REM Outputs "War And Peace"
```
