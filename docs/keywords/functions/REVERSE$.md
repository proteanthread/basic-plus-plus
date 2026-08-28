# `REVERSE$` String Character Reversal Function

## 1. BASIC Usage and Function Definition

The `REVERSE$` function reverses the sequence of characters in a string expression.

### Syntax Signatures:
```basic
reversed$ = REVERSE$(string_expression$)
```

### Operational Rules:
- Reverses character order from end to start.
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 PRINT REVERSE$("stressed") : REM Outputs "desserts"
```
