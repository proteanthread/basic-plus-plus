# `XOR` Logical and Bitwise Exclusive-OR Operator (Universal Dual-Format)

## 1. BASIC Usage and Operator/Function Definition

The `XOR` keyword performs bitwise exclusive OR across all 64 bits of integer operands. It is supported in both **infix** and **prefix** notation.

### Syntax Signatures:
```basic
result = expr1 XOR expr2
result = XOR(expr1, expr2)
```

### Operational Rules:
- Performs bitwise XOR on 64-bit integer values: `(int64_t)expr1 ^ (int64_t)expr2`.
- In RPN stack evaluation blocks: `{ expr1 expr2 XOR }`.

---

## 2. Code Examples

```basic
10 PRINT "Infix: "; 255 XOR 15 : REM Outputs 240
20 PRINT "Prefix: "; XOR(255, 15) : REM Outputs 240
```
