# `DEF_FN` User Defined Function Statement

## 1. BASIC Usage and Keyword Definition

Defines a single-line user function that can be evaluated anywhere in expressions via FNname().

### Syntax Signatures:
```basic
DEF FNname[(arg1 [, arg2...])] = expression
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Invalid argument count or recursive definition.

### Operational Notes:
- Single-line expression macro evaluated in current variable scope.

---

## 2. Code Examples

```basic
10 DEF FNHypot(A, B) = SQR(A * A + B * B)
20 PRINT "Hypotenuse: "; FNHypot(3, 4) : REM Outputs 5
```
