# `NEXT` End of FOR Loop Statement

## 1. BASIC Usage and Keyword Definition

Increments the loop counter variable and branches back to the matching FOR statement if not complete.

### Syntax Signatures:
```basic
NEXT [variable [, variable...]]
```

### Error Handling & Boundary Conditions:
- **Error 1 (ERR_NEXT_WITHOUT_FOR)**: No matching FOR statement active.

### Operational Notes:
- Supports multiple nested counters in single statement (e.g. NEXT J, I).

---

## 2. Code Examples

```basic
10 FOR I = 1 TO 5
20   PRINT I
30 NEXT I
```
