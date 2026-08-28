# `RESETBIT` Bit Clear Operator (Universal Dual-Format)

## 1. BASIC Usage and Function/Operator Definition

The `RESETBIT` keyword clears a specific bit position (0 to 63) of a 64-bit integer to 0. It is supported in both **infix** and **prefix** notation.

### Syntax Signatures:
```basic
result = value RESETBIT bit_pos
result = RESETBIT(value, bit_pos)
```

### Operational Rules:
- Returns `(uint64_t)value & ~(1ULL << (uint64_t)bit_pos)`.
- In RPN stack evaluation blocks: `{ value bit_pos RESETBIT }`.

---

## 2. Code Examples

```basic
10 PRINT "Infix: "; 1024 RESETBIT 10 : REM Outputs 0
20 PRINT "Prefix: "; RESETBIT(1024, 10) : REM Outputs 0
```
