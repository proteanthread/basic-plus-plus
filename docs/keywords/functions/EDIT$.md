# `EDIT$` String Editing & Transformation Function

## 1. BASIC Usage and Function Definition

The `EDIT$` function performs bitmask-controlled string transformations (stripping control chars, trimming whitespace, collapsing multiple spaces, uppercasing) on a source string.

### Syntax Signatures:
```basic
res$ = EDIT$(source_string$, control_mask%)
```

### Operational Rules:
- Bit 1: Discard parity bit.
- Bit 2: Discard all spaces and tabs.
- Bit 4: Discard carriage returns, line feeds, nulls.
- Bit 8: Discard leading spaces and tabs.
- Bit 16: Collapse multiple spaces to single spaces.
- Bit 32: Convert lowercase to uppercase.
- Bit 128: Discard trailing spaces and tabs.

---

## 2. Code Examples

```basic
10 S$ = "   Hello   World   "
20 PRINT EDIT$(S$, 32 + 8 + 128) : REM Outputs "HELLO   WORLD"
```
