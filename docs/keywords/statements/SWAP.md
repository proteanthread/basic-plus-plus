# `SWAP` Variable Exchange Statement

## 1. BASIC Usage and Keyword Definition

The `SWAP` statement exchanges the values of two variables or array elements of identical data types without requiring a temporary third variable.

### Syntax Signatures:
```basic
SWAP variable1, variable2
SWAP array(i), array(j)
```

### Operational Rules:
- **Type Requirement**: Both operands must have identical types (both numeric, or both string). Swapping mismatched types raises Error 13 (`ERR_TYPE_MISMATCH`).
- **String Safety**: Exchanges reference-counted string pointers safely without double-freeing or memory leaks.

---

## 2. Code Examples

```basic
10 A$ = "Hello" : B$ = "World"
20 PRINT "Before: A="; A$; " B="; B$
30 SWAP A$, B$
40 PRINT "After : A="; A$; " B="; B$
```
