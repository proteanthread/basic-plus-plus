# `REVERSE` Reverse Video Display Toggle Statement

## 1. BASIC Usage and Keyword Definition

Toggles reverse video display attribute mode on virtual console devices.

### Syntax Signatures:
```basic
REVERSE flag%
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Flag must be 0 or 1.

### Operational Notes:
- ANSI video attribute control.

---

## 2. Code Examples

```basic
10 REVERSE 1
20 PRINT "Highlighted Text"
30 REVERSE 0
```
