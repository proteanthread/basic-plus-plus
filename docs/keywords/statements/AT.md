# `AT` Screen Cursor Positioning Clause

## 1. BASIC Usage and Keyword Definition

Specifies 1-based text cursor coordinates directly within PRINT statements.

### Syntax Signatures:
```basic
PRINT AT row%, col%; expression
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Coordinates outside screen bounds.

### Operational Notes:
- Common in Sinclair ZX Spectrum and Locomotive BASIC dialects.

---

## 2. Code Examples

```basic
10 PRINT AT 10, 20; "Hello at Row 10 Col 20"
```
