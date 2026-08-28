# `PRINT` Formatted Console and File Output Statement

## 1. BASIC Usage and Keyword Definition

Outputs text, numeric values, and expressions to the screen console, line printer, or open file channel.

### Syntax Signatures:
```basic
PRINT [#filenum%,] [USING format_string$;] [expression [{; | ,}] ...]
```

### Error Handling & Boundary Conditions:
- **Error 52 (ERR_BAD_FILE_NUMBER)**: File channel not open for output.

### Operational Notes:
- Semicolon suppresses trailing newline; comma advances to next tab stop (14 cols).

---

## 2. Code Examples

```basic
10 PRINT "Name", "Score"
20 PRINT "Alice"; 95
30 PRINT USING "Total: $$###.##"; 49.99
```
