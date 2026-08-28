# `CSNG` Convert to Single-Precision Float

## 1. BASIC Usage and Function Definition

The `CSNG` function converts a numeric expression or numeric string into a 32-bit single-precision floating-point value (`VAL_NUMBER` rounded to single-precision float precision).

### Syntax Signatures:
```basic
result! = CSNG(expression)
```

### Operational Rules:
- Converts integers or double-precision floats to 32-bit IEEE 754 single precision (~6-7 significant decimal digits).
- String arguments containing numeric text are parsed and converted.

---

## 2. Code Examples

```basic
10 DBL# = 3.141592653589793
20 SNG! = CSNG(DBL#)
30 PRINT "Double: "; DBL#
40 PRINT "Single: "; SNG!
```
