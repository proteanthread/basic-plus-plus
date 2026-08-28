# `OUT` Hardware I/O Port Output Statement

## 1. BASIC Usage and Keyword Definition

Writes an 8-bit byte value to a specified hardware or virtualized I/O port address.

### Syntax Signatures:
```basic
OUT port_address%, byte_value%
```

### Error Handling & Boundary Conditions:
- **Error 70 (ERR_PERMISSION_DENIED)**: Hardware direct I/O blocked in sandbox.

### Operational Notes:
- Routes to virtual BIOS and device bus drivers.

---

## 2. Code Examples

```basic
10 OUT &H3C8, 0 : REM Set VGA palette index 0
20 OUT &H3C9, 63 : OUT &H3C9, 0 : OUT &H3C9, 0 : REM Full red
```
