# `MCASE$` Mixed / Matching Case String Transformation Function

## 1. BASIC Usage and Function Definition

The `MCASE$` function applies mixed case capitalization (such as Title Case or alternating casing) across a target string.

### Syntax Signatures:
```basic
res$ = MCASE$(string_expression$)
```

### Operational Rules:
- Capitalizes the initial letter of words while lowering subsequent characters.
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 PRINT MCASE$("the quick brown fox") : REM Outputs "The Quick Brown Fox"
```
