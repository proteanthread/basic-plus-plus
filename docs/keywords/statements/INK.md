# `INK` Foreground Pen Color Setting Statement

## 1. BASIC Usage and Keyword Definition

Sets the active foreground drawing and text color in Sinclair and Locomotive BASIC dialects.

### Syntax Signatures:
```basic
INK color_index%
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Color index outside range.

### Operational Notes:
- Retains compatibility with ZX Spectrum display hardware.

---

## 2. Code Examples

```basic
10 INK 2 : REM Set red ink
20 PRINT "Red text"
```
