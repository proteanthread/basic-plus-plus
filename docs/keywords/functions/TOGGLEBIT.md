# `TOGGLEBIT` Bit Inversion Operator (Universal Dual-Format)

## 1. BASIC Usage and Function/Operator Definition

The `TOGGLEBIT` keyword inverts a specific bit position (0 to 63) of a 64-bit integer. It is supported in both **infix** and **prefix** notation.

### Syntax Signatures:
```basic
result = value TOGGLEBIT bit_pos
result = TOGGLEBIT(value, bit_pos)
```

### Operational Rules:
- Returns `(uint64_t)value ^ (1ULL << (uint64_t)bit_pos)`.
- In RPN stack evaluation blocks: `{ value bit_pos TOGGLEBIT }`.

---

## 2. Code Examples

```basic
10 PRINT "Infix: "; 0 TOGGLEBIT 8 : REM Outputs 256
20 PRINT "Prefix: "; TOGGLEBIT(256, 8) : REM Outputs 0
```
