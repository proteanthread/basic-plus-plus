# `BRIGHT` Retro Color Intensity Attribute Statement

## 1. BASIC Usage and Keyword Definition

Toggles high-intensity color palette rendering in retro display modes.

### Syntax Signatures:
```basic
BRIGHT intensity_flag%
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Flag must be 0 or 1.

### Operational Notes:
- ZX Spectrum display attribute emulation.

---

## 2. Code Examples

```basic
10 BRIGHT 1 : REM Enable bright attributes
20 PRINT "High intensity text"
```
