# `OR` Logical and Bitwise Disjunction Operator (Universal Dual-Format)

## 1. BASIC Usage and Operator/Function Definition

The `OR` keyword performs bitwise integer disjunction across all 64 bits of integer operands, or logical disjunction on boolean expressions. It is supported in both **infix** and **prefix/variadic** notation.

### Syntax Signatures:
```basic
result = expr1 OR expr2
result = OR(expr1, expr2 [, expr3, ...])
```

### Operational Rules:
- Performs bitwise OR across all 64 bits of integer operands.
- Supports 2 or more operands in functional prefix notation.
- In RPN stack evaluation blocks: `{ expr1 expr2 OR }`.

---

## 2. Code Examples

```basic
10 MASK = 16 OR 8 : REM Infix: Outputs 24
20 PRINT "Prefix: "; OR(16, 8, 4) : REM Outputs 28
```
