# `MOD` Modulo Remainder Operator (Universal Dual-Format)

## 1. BASIC Usage and Operator/Function Definition

The `MOD` keyword computes the integer remainder of division between two numeric expressions. It is supported in both **infix** and **prefix** notation.

### Syntax Signatures:
```basic
remainder = expr1 MOD expr2
remainder = MOD(expr1, expr2)
```

### Operational Rules:
- Computes `(int64_t)expr1 % (int64_t)expr2`.
- In RPN stack evaluation blocks: `{ expr1 expr2 MOD }`.
- Division by zero returns `Error 11: Division by zero`.

---

## 2. Code Examples

```basic
10 PRINT "Infix: "; 17 MOD 5 : REM Outputs 2
20 PRINT "Prefix: "; MOD(17, 5) : REM Outputs 2
```
