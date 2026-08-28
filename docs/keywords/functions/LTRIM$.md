# `LTRIM$` Left-Trim Whitespace Function

## 1. BASIC Usage and Function Definition

The `LTRIM$` function returns a copy of a string with all leading spaces and horizontal tab characters removed.

### Syntax Signatures:
```basic
trimmed$ = LTRIM$(string_expression$)
```

### Operational Rules:
- Strips leading ASCII space (`32`) and tab (`9`) characters.
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 S$ = "   BASIC++   "
20 PRINT "["; LTRIM$(S$); "]" : REM Outputs "[BASIC++   ]"
```
