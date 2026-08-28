# `COMP` String Comparison Function

## 1. BASIC Usage and Function Definition

The `COMP` function performs a three-way lexical comparison between two string expressions.

### Syntax Signatures:
```basic
res% = COMP(str1$, str2$)
```

### Operational Rules:
- Returns $-1$ if $\text{str1} < \text{str2}$ lexicographically.
- Returns $0$ if $\text{str1} = \text{str2}$.
- Returns $+1$ if $\text{str1} > \text{str2}$.

---

## 2. Code Examples

```basic
10 PRINT COMP("apple", "banana") : REM Outputs -1
20 PRINT COMP("cherry", "cherry") : REM Outputs 0
```
