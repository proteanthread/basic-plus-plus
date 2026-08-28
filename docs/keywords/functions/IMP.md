# `IMP` Logical and Bitwise Implication Operator (Universal Dual-Format)

## 1. BASIC Usage and Operator/Function Definition

The `IMP` keyword performs bitwise implication on 64-bit integer operands or logical implication on boolean expressions. It is supported in both **infix** and **prefix** notation.

### Syntax Signatures:
```basic
result = expr1 IMP expr2
result = IMP(expr1, expr2)
```

### Operational Rules:
- Performs bitwise implication: `~(int64_t)expr1 | (int64_t)expr2`.
- In RPN stack evaluation blocks: `{ expr1 expr2 IMP }`.

---

## 2. Code Examples

```basic
10 PRINT "Infix: "; 0 IMP 1 : REM Outputs -1 (True)
20 PRINT "Prefix: "; IMP(0, 1) : REM Outputs -1 (True)
```
