# `HEX$` Hexadecimal String Conversion Function

## 1. BASIC Usage and Function Definition

The `HEX$` function converts an integer expression into its base-16 hexadecimal text representation.

### Syntax Signatures:
```basic
hex_str$ = HEX$(numeric_expression)
```

### Operational Rules:
- Returns an uppercase string containing characters `0-9` and `A-F`.
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 PRINT HEX$(255)  : REM Outputs "FF"
20 PRINT HEX$(4096) : REM Outputs "1000"
```
