# `LET` Variable Assignment Statement

## 1. BASIC Usage and Keyword Definition

The `LET` statement assigns the value of an expression to a scalar variable, an array element, or multiple variables in a chained assignment. In BASIC++, the `LET` keyword is optional for standard assignments (e.g., `LET A = 10` is identical to `A = 10`).

### Syntax Signatures:
```basic
[LET] variable = expression
[LET] array(index1 [, index2, ...]) = expression
[LET] var1, var2, var3 = expression   : REM Multi-target assignment
```

### Operational Rules:
- **Optional Keyword**: The `LET` keyword can be omitted in modern and classic programs.
- **Type Compatibility**: Numeric expressions cannot be assigned to string variables, and string expressions cannot be assigned to numeric variables (triggers Error 13: `ERR_TYPE_MISMATCH`).
- **String Memory**: When assigning strings, reference counts are updated cleanly in `VariableContext`.

---

## 2. Code Examples

```basic
10 LET X = 100
20 Y = 200 : REM Implicit LET
30 LET TOTAL = X + Y
40 PRINT "Total: "; TOTAL
```
