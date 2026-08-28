# `CDBL` Convert to Double-Precision Float Function

## 1. BASIC Usage and Function Definition

The `CDBL` function explicitly converts any numeric expression into a 64-bit IEEE 754 double-precision floating-point number.

### Syntax Signatures:
```basic
dbl# = CDBL(numeric_expression)
```

### Operational Rules:
- Promotes integer, single-precision, or byte values to 64-bit float representation.

---

## 2. Code Examples

```basic
10 A% = 42
20 B# = CDBL(A%)
30 PRINT "Double value: "; B#
```
