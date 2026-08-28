# `AND` Logical and Bitwise Conjunction Operator (Universal Dual-Format)

## 1. BASIC Usage and Operator/Function Definition

The `AND` keyword performs bitwise integer conjunction across all 64 bits of integer operands, or logical conjunction on boolean expressions. It is supported in both **infix** notation and **prefix/variadic** functional notation.

### Syntax Signatures:
```basic
result = expr1 AND expr2
result = AND(expr1, expr2 [, expr3, ...])
```

### Operational Rules:
- Performs bitwise AND across all 64 bits of integer operands.
- Supports 2 or more operands in functional prefix notation.
- In RPN stack evaluation blocks: `{ expr1 expr2 AND }`.

---

## 2. Code Examples

```basic
10 MASK = &H0F AND &HFF : REM Infix: Outputs 15 (&H0F)
20 PRINT "Prefix: "; AND(255, 15, 7) : REM Outputs 7
30 IF (A > 0) AND (B > 0) THEN PRINT "Both positive."
```
