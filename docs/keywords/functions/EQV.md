# `EQV` Logical and Bitwise Equivalence Operator (Universal Dual-Format)

## 1. BASIC Usage and Operator/Function Definition

The `EQV` keyword performs bitwise equivalence on 64-bit integer operands or logical equivalence on boolean expressions. It is supported in both **infix** and **prefix** notation.

### Syntax Signatures:
```basic
result = expr1 EQV expr2
result = EQV(expr1, expr2)
```

### Operational Rules:
- Performs bitwise equivalence: `~((int64_t)expr1 ^ (int64_t)expr2)`.
- In RPN stack evaluation blocks: `{ expr1 expr2 EQV }`.

---

## 2. Code Examples

```basic
10 PRINT "Infix: "; 15 EQV 15 : REM Outputs -1 (True)
20 PRINT "Prefix: "; EQV(15, 15) : REM Outputs -1 (True)
```
