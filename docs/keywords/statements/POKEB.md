# `POKEB` Write Banked Memory Byte Statement

## 1. BASIC Usage and Keyword Definition

The `POKEB` statement writes an 8-bit unsigned byte value ($0$ to $255$) directly into an explicit 64KB memory bank and address offset without altering the global `BANK` register.

### Syntax Signatures:
```basic
POKEB bank_number%, offset_address, byte_value%
```

### Operational Rules:
- **Explicit Banking**: Directly targets the specified bank without modifying the active `BANK` selection.
- **Value Clamping/Validation**: `byte_value` must be between $0$ and $255$.
- **Security Check**: Enforced via `CAP_MEM` security permissions.

---

## 2. Code Examples

```basic
10 POKEB 1, &H1000, 42
20 PRINT "Value written to Bank 1: "; PEEKB(1, &H1000)
```
