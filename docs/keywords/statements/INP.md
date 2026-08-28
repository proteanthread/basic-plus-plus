# `INP` Hardware I/O Port Input Function

## 1. BASIC Usage and Keyword Definition

Reads a single 8-bit byte value from the specified hardware or virtualized I/O port address.

### Syntax Signatures:
```basic
byte_val% = INP(port_address%)
```

### Error Handling & Boundary Conditions:
- **Error 70 (ERR_PERMISSION_DENIED)**: Direct I/O disabled in secure sandbox mode.

### Operational Notes:
- Virtual BIOS and device bus trap handler routing.

---

## 2. Code Examples

```basic
10 STATUS% = INP(&H3DA) : REM Read CGA status register
```
