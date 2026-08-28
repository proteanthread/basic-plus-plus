# `INVERSE` Reverse Video Display Mode Statement

## 1. BASIC Usage and Keyword Definition

Toggles reverse video text rendering (swapping foreground and background colors).

### Syntax Signatures:
```basic
INVERSE inverse_flag%
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Flag must be 0 or 1.

### Operational Notes:
- Apple II and ZX Spectrum display attribute compatibility.

---

## 2. Code Examples

```basic
10 INVERSE 1
20 PRINT " REVERSE HIGHLIGHT "
30 INVERSE 0
```
