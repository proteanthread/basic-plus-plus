# `REPLACE$` Substring Replacement Function

## 1. BASIC Usage and Function Definition

The `REPLACE$` function searches a source string for occurrences of a search string and replaces them with a replacement string.

### Syntax Signatures:
```basic
res$ = REPLACE$(source$, search$, replacement$)
```

### Operational Rules:
- Replaces all non-overlapping occurrences of `search$` in `source$`.
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 S$ = "Hello World"
20 PRINT REPLACE$(S$, "World", "BASIC++") : REM Outputs "Hello BASIC++"
```
