# `WAIT` Hardware Port Bitmask Wait Statement

## 1. BASIC Usage and Keyword Definition

Suspends program execution until reading the specified hardware port satisfies: (INP(port) XOR xor_mask) AND and_mask != 0.

### Syntax Signatures:
```basic
WAIT port_address%, and_mask% [, xor_mask%]
```

### Error Handling & Boundary Conditions:
- **Error 70 (ERR_PERMISSION_DENIED)**: Port I/O blocked in sandbox mode.

### Operational Notes:
- Enables hardware synchronization without CPU spin loops.

---

## 2. Code Examples

```basic
10 WAIT &H3DA, 8 : REM Wait for vertical retrace on CGA/VGA
```
