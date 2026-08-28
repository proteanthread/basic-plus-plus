# `WIDTH` Screen and Printer Column Width Statement

## 1. BASIC Usage and Keyword Definition

Sets the number of text columns (e.g. 40 or 80) and rows (25, 43, 50) for the screen display, printer, or file channel.

### Syntax Signatures:
```basic
WIDTH [columns%] [, rows%]
WIDTH [#]filenum%, columns%
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Width/height not supported by active display.

### Operational Notes:
- Controls automatic text wrapping margins.

---

## 2. Code Examples

```basic
10 WIDTH 80, 25 : REM Standard 80x25 text
20 WIDTH 40, 25 : REM 40-column large text mode
```
