# `PAPER` Background Paper Color Setting Statement

## 1. BASIC Usage and Keyword Definition

Sets the active background paper color for text output in Sinclair and Locomotive BASIC dialects.

### Syntax Signatures:
```basic
PAPER color_index%
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Color index outside range.

### Operational Notes:
- ZX Spectrum display attribute emulation.

---

## 2. Code Examples

```basic
10 PAPER 1 : REM Set blue background
20 INK 7   : REM Set white text
30 CLS
40 PRINT "White on Blue"
```
