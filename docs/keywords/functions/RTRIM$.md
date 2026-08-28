# `RTRIM$` Right-Trim Whitespace Function

## 1. BASIC Usage and Function Definition

The `RTRIM$` function returns a copy of a string with all trailing spaces and tab characters removed.

### Syntax Signatures:
```basic
trimmed$ = RTRIM$(string_expression$)
```

### Operational Rules:
- Strips trailing spaces (`32`) and tabs (`9`).
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 S$ = "   BASIC++   "
20 PRINT "["; RTRIM$(S$); "]" : REM Outputs "[   BASIC++]"
```
