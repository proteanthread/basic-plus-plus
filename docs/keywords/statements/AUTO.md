# `AUTO` Automatic Line Numbering Command

## 1. BASIC Usage and Keyword Definition

Automatically generates line numbers during interactive program editing in console mode.

### Syntax Signatures:
```basic
AUTO [start_line% [, increment%]]
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Invalid line number or increment <= 0.

### Operational Notes:
- Interactive command available in baspp and bpp REPLs.

---

## 2. Code Examples

```basic
AUTO 100, 10  : REM Generates lines 100, 110, 120...
```
