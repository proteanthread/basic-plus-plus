# `LEFT$` Left Substring Extraction Function

## 1. BASIC Usage and Function Definition

The `LEFT$` function extracts the leftmost $N$ characters from a string expression.

### Syntax Signatures:
```basic
sub$ = LEFT$(string_expression$, count%)
```

### Operational Rules:
- If `count% <= 0`, returns empty string `""`.
- If `count% >= LEN(string_expression$)`, returns full string.
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 S$ = "RetroBASIC"
20 PRINT LEFT$(S$, 5) : REM Outputs "Retro"
```
