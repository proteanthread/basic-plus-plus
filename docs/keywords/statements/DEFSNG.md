# `DEFSNG` Define Single-Precision Default Type

## 1. BASIC Usage and Keyword Definition

The `DEFSNG` statement declares that all variable identifiers beginning with the specified letter(s) or letter ranges default to single-precision floating-point numbers (`!` / 32-bit float) when declared or referenced without an explicit type suffix.

### Syntax Signatures:
```basic
DEFSNG letter_range [, letter_range2, ...]
```

### Operational Rules:
- Declares that un-suffixed variables starting with the given letters will be stored as single-precision floats.
- Overridden by explicit suffixes (`%`, `#`, `$`).

---

## 2. Code Examples

```basic
10 DEFSNG A-Z
20 X = 3.141592653589793
30 PRINT "Single precision value: "; X
```
