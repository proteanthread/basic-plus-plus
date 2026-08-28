# `LEN` String Length and Variable Size Function

## 1. BASIC Usage and Function Definition

The `LEN` function returns the number of characters in a string expression, or the size in bytes of a user-defined record structure variable.

### Syntax Signatures:
```basic
length% = LEN(string_expression$)
size_bytes% = LEN(variable)
```

### Operational Rules:
- Returns integer character count $\ge 0$.

---

## 2. Code Examples

```basic
10 S$ = "BASIC++"
20 PRINT "Length: "; LEN(S$) : REM Outputs 7
```
