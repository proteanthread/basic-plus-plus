# `GOTO` Unconditional Control Transfer Statement

## 1. BASIC Usage and Keyword Definition

Unconditionally transfers execution control to the designated program line number or symbolic label.

### Syntax Signatures:
```basic
GOTO {line_number% | @label}
```

### Error Handling & Boundary Conditions:
- **Error 8 (ERR_UNDEFINED_LINE_NUMBER)**: Target line number does not exist.

### Operational Notes:
- Fast line pointer redirection in VM execution loop.

---

## 2. Code Examples

```basic
10 PRINT "Step 1"
20 GOTO 40
30 PRINT "Skipped"
40 PRINT "Step 2"
```
