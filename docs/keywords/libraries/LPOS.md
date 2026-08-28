# `LPOS` Line Printer Column Position Function

## 1. BASIC Usage and Function Definition

The `LPOS` function returns the current print head column position of the designated line printer buffer (`LPT1:`, `LPT2:`, `LPT3:`).

### Syntax Signatures:
```basic
col% = LPOS(printer_number%)
```

### Operational Rules:
- Returns the 1-based horizontal column position of the printer line buffer ($1 \le \text{col} \le 255$).
- `printer_number` specifies the printer device index ($0$ or $1$ for `LPT1:`, $2$ for `LPT2:`, $3$ for `LPT3:`).

---

## 2. Code Examples

```basic
10 LPRINT "Invoice Number: 1042";
20 PRINT "Printer column position: "; LPOS(1)
```
