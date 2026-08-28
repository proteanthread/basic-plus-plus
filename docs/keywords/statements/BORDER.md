# `BORDER` Screen Border Color Attribute Statement

## 1. BASIC Usage and Keyword Definition

Sets the color of the outer screen border on supported graphics modes.

### Syntax Signatures:
```basic
BORDER color_index%
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Color index out of range for active screen mode.

### Operational Notes:
- Full support in Commodore, ZX Spectrum, and Amstrad display modes.

---

## 2. Code Examples

```basic
10 BORDER 1 : REM Set blue border
20 COLOR 14, 0 : CLS
```
