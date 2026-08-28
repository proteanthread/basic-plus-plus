# `NOT` Logical and Bitwise Complement Operator (Universal Dual-Format)

## 1. BASIC Usage and Operator/Function Definition

The `NOT` keyword performs bitwise complement across all 64 bits of integer operands or logical negation of boolean expressions. It is supported in both **unary/prefix** (`NOT expr`) and **functional prefix** (`NOT(expr)`) notation.

### Syntax Signatures:
```basic
result = NOT expr
result = NOT(expr)
```

### Operational Rules:
- Performs bitwise complement: `~(int64_t)expr`.
- In RPN stack evaluation blocks: `{ expr NOT }`.

---

## 2. Code Examples

```basic
10 PRINT "Unary: "; NOT 0 : REM Outputs -1
20 PRINT "Functional: "; NOT(0) : REM Outputs -1
```
