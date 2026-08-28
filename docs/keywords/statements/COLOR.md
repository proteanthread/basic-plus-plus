# `COLOR` Color Attribute Configuration Statement

## 1. BASIC Usage and Keyword Definition

Sets active text foreground, background, and screen border color attributes.

### Syntax Signatures:
```basic
COLOR [foreground%] [, [background%] [, border%]]
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Color index outside palette range.

### Operational Notes:
- Supports standard 16-color ANSI/CGA palettes and 256-color extended modes.

---

## 2. Code Examples

```basic
10 COLOR 15, 1 : REM Bright white on blue
20 CLS
30 PRINT "White on Blue Header"
```
