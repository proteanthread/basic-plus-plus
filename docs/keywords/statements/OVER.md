# `OVER` Transparent Text Overlay Attribute Statement

## 1. BASIC Usage and Keyword Definition

Toggles transparent text background overlay mode (printing text without erasing background pixels).

### Syntax Signatures:
```basic
OVER over_flag%
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Flag must be 0 or 1.

### Operational Notes:
- Sinclair ZX Spectrum and Locomotive BASIC attribute emulation.

---

## 2. Code Examples

```basic
10 SCREEN 12 : CLS
20 LINE (10, 10)-(200, 100), 2, BF
30 OVER 1
40 PRINT "Text over graphics"
50 OVER 0
```
