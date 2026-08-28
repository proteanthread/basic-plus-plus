# `SHL` Bitwise Shift Left (Universal Dual-Format)

## 1. BASIC Usage and Function/Operator Definition

The `SHL` keyword performs a bitwise left shift on a 64-bit integer value by a specified number of bit positions (0 to 63). It is fully supported in both **infix** notation and **functional/prefix** notation with identical semantics.

### Syntax Signatures:
```basic
result = value SHL shift_count
result = SHL(value, shift_count)
```

### Operational Rules:
- Performs a 64-bit bitwise left shift: `(uint64_t)value << (uint64_t)shift_count`.
- Bits shifted past position 63 are discarded, and zeros are shifted into low-order positions.
- In RPN stack evaluation blocks: `{ value shift_count SHL }`.
- `shift_count` must be in the range 0 to 63; values outside this range return `Error 5: Illegal Function Call`.

---

## 2. Code Examples

```basic
10 REM Infix notation
20 PRINT "1 SHL 4 = "; 1 SHL 4 : REM Outputs 16
30 REM Functional prefix notation
40 PRINT "SHL(1, 35) = "; SHL(1, 35) : REM Outputs 34359738368
50 REM RPN evaluation
60 PRINT "RPN: "; { 1 10 SHL } : REM Outputs 1024
```
