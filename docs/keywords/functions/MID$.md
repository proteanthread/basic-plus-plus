# `MID$` Mid-String Extraction & Replacement Function

## 1. BASIC Usage and Function Definition

The `MID$` function extracts a substring of designated length starting at a 1-based character offset from a source string.

### Syntax Signatures:
```basic
sub$ = MID$(source_string$, start_pos% [, length%])
MID$(target_var$, start_pos% [, length%]) = replacement$
```

### Operational Rules:
- `start_pos%` is 1-based. If `start_pos% > LEN(source_string$)`, returns empty string `""`.
- If `length%` is omitted, extracts to the end of the string.
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 S$ = "HELLO WORLD"
20 PRINT MID$(S$, 7, 5) : REM Outputs "WORLD"
```
