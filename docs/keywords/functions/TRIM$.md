# `TRIM$` Whitespace Trimming Function

## 1. BASIC Usage and Function Definition

The `TRIM$` function returns a copy of a string with all leading and trailing space and tab characters removed.

### Syntax Signatures:
```basic
trimmed$ = TRIM$(string_expression$)
```

### Operational Rules:
- Strips leading and trailing ASCII spaces (`32`) and tabs (`9`).
- Equivalent to `LTRIM$(RTRIM$(string_expression$))`.
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 S$ = "   BASIC++ Engine   "
20 PRINT "["; TRIM$(S$); "]" : REM Outputs "[BASIC++ Engine]"
```
