# `HI` High Byte / Word Extractor Function

## 1. BASIC Usage and Function Definition

The `HI` (or `HIBYTE`) function extracts the high-order byte (bits 8..15) or high word from an integer expression.

### Syntax Signatures:
```basic
high_byte% = HI(numeric_expression)
```

### Operational Rules:
- Computes `(numeric_expression \ 256) AND 255`.

---

## 2. Code Examples

```basic
10 VAL% = &H1234
20 PRINT "High byte: "; HEX$(HI(VAL%)) : REM Outputs "12"
```
