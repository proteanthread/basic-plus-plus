# `SHR` Bitwise Shift Right (Universal Dual-Format)

## 1. BASIC Usage and Function/Operator Definition

The `SHR` keyword performs a bitwise right shift on a 64-bit integer value by a specified number of bit positions (0 to 63). It is fully supported in both **infix** notation and **functional/prefix** notation with identical semantics.

### Syntax Signatures:
```basic
result = value SHR shift_count
result = SHR(value, shift_count)
```

### Operational Rules:
- Performs a 64-bit bitwise right shift: `(uint64_t)value >> (uint64_t)shift_count`.
- In RPN stack evaluation blocks: `{ value shift_count SHR }`.
- `shift_count` must be in the range 0 to 63; values outside this range return `Error 5: Illegal Function Call`.

---

## 2. Code Examples

```basic
10 REM Infix notation
20 PRINT "1024 SHR 4 = "; 1024 SHR 4 : REM Outputs 64
30 REM Functional prefix notation
40 PRINT "SHR(34359738368, 5) = "; SHR(34359738368, 5) : REM Outputs 1073741824
50 REM RPN evaluation
60 PRINT "RPN: "; { 256 3 SHR } : REM Outputs 32
```
