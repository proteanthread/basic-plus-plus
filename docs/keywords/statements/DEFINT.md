# `DEFINT` Define Integer Default Type

## 1. BASIC Usage and Keyword Definition

The `DEFINT` statement declares that all variable identifiers beginning with the specified letter(s) or letter ranges default to integer variables (`%` / 64-bit signed integers in standard BASIC++) when declared or referenced without an explicit type suffix.

### Syntax Signatures:
```basic
DEFINT letter_range [, letter_range2, ...]
```

### Operational Rules:
- Declares that un-suffixed variables starting with the given letters will be stored as integer values.
- Truncates/rounds floating-point assignments to integer precision.
- Overridden by explicit suffixes (`!`, `#`, `$`).

---

## 2. Code Examples

```basic
10 DEFINT I-N : REM Fortran/BASIC classic integer convention
20 I = 42 : J = 10
30 PRINT "I / J = "; I \ J; " (integer division)"
```
