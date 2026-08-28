# `UCASE$` Uppercase String Conversion Function

## 1. BASIC Usage and Function Definition

The `UCASE$` function converts all lowercase alphabetic characters in a string to uppercase.

### Syntax Signatures:
```basic
upper_str$ = UCASE$(string_expression$)
```

### Operational Rules:
- Converts ASCII `'a'-'z'` to `'A'-'Z'`.
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 S$ = "basic++ master edition"
20 PRINT UCASE$(S$) : REM Outputs "BASIC++ MASTER EDITION"
```
