# `PEEKB` Read Banked Memory Byte Function

## 1. BASIC Usage and Function Definition

The `PEEKB` function reads and returns an 8-bit unsigned byte value ($0$ to $255$) from an explicit 64KB memory bank and address offset without altering the global `BANK` register.

### Syntax Signatures:
```basic
byte_val% = PEEKB(bank_number%, offset_address)
```

### Operational Rules:
- **Explicit Banking**: Combines bank index and offset into a single call.
- **Return Range**: Returns $0 \le \text{byte} \le 255$.
- **Security Check**: Verified via `CAP_MEM` capability flags.

---

## 2. Code Examples

```basic
10 BYTE_VAL = PEEKB(1, &H1000)
20 PRINT "Bank 1, Offset &H1000 value: "; BYTE_VAL
```
