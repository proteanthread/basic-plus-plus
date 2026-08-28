# `LO` Low Byte / Word Extractor Function

## 1. BASIC Usage and Function Definition

The `LO` (or `LOBYTE`) function extracts the low-order byte (bits 0..7) from an integer expression.

### Syntax Signatures:
```basic
low_byte% = LO(numeric_expression)
```

### Operational Rules:
- Computes `numeric_expression AND 255`.

---

## 2. Code Examples

```basic
10 VAL% = &H1234
20 PRINT "Low byte: "; HEX$(LO(VAL%)) : REM Outputs "34"
```
