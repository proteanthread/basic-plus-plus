# `CINT` Convert to Integer Function

## 1. BASIC Usage and Function Definition

The `CINT` function converts a numeric expression to an integer by rounding the fractional portion to the nearest whole integer.

### Syntax Signatures:
```basic
result% = CINT(numeric_expression)
```

### Operational Rules:
- **Round-to-Nearest**: Rounds fractional values to nearest integer (e.g. `CINT(4.6)` = `5`, `CINT(-4.6)` = `-5`).
- **Legacy Bounds**: In 16-bit GW-BASIC compatibility mode, values outside $-32768$ to $32767$ raise Error 6 (`ERR_OVERFLOW`). In standard 64-bit BASIC++, supports full integer range.

---

## 2. Code Examples

```basic
10 FOR X = 1.0 TO 2.0 STEP 0.3
20   PRINT "X = "; X; " -> CINT(X) = "; CINT(X)
30 NEXT X
```
