# `WEND` End of WHILE Loop Statement

## 1. BASIC Usage and Keyword Definition

Marks the end of a WHILE loop block, branching back to the matching WHILE condition test.

### Syntax Signatures:
```basic
WEND
```

### Error Handling & Boundary Conditions:
- **Error 2 (ERR_SYNTAX_ERROR)**: WEND without matching WHILE.

### Operational Notes:
- Classic structured loop construct.

---

## 2. Code Examples

```basic
10 X = 1
20 WHILE X <= 5
30   PRINT X
40   X = X + 1
50 WEND
```
