# `PEN` Light Pen Status Query Function

## 1. BASIC Usage and Keyword Definition

Returns coordinates, trigger status, and character positions from the light pen input device.

### Syntax Signatures:
```basic
status% = PEN(function_code%)
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Function code outside range (0..9).

### Operational Notes:
- Emulated through mouse click and position events.

---

## 2. Code Examples

```basic
10 IF PEN(0) THEN PRINT "Pen down at: "; PEN(1); ", "; PEN(2)
```
