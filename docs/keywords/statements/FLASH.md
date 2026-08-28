# `FLASH` Retro Text Blinking Attribute Statement

## 1. BASIC Usage and Keyword Definition

Toggles text character blinking mode in retro and ANSI display systems.

### Syntax Signatures:
```basic
FLASH flash_flag%
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Flag must be 0 or 1.

### Operational Notes:
- ZX Spectrum attribute emulation.

---

## 2. Code Examples

```basic
10 FLASH 1
20 PRINT "WARNING: SYSTEM OVERLOAD"
30 FLASH 0
```
