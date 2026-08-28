# `CLS` Screen Clearing Statement

## 1. BASIC Usage and Keyword Definition

Clears the active text or graphics viewport and moves the cursor to (1,1).

### Syntax Signatures:
```basic
CLS [mode%]
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Invalid clear mode.

### Operational Notes:
- Mode 0 clears text and graphics viewports; mode 1 clears graphics only; mode 2 clears text only.

---

## 2. Code Examples

```basic
10 CLS : REM Clear entire screen
20 PRINT "Top left corner"
```
