# `WRITE` Delimited Output Statement

## 1. BASIC Usage and Keyword Definition

Outputs expressions to the screen or a file, separating items with commas and enclosing strings in quotation marks.

### Syntax Signatures:
```basic
WRITE [#filenum%,] expression [, expression...]
```

### Error Handling & Boundary Conditions:
- **Error 52 (ERR_BAD_FILE_NUMBER)**: File channel is not open.

### Operational Notes:
- Produces valid CSV format data for subsequent INPUT statements.

---

## 2. Code Examples

```basic
10 OPEN "O", #1, "DATA.CSV"
20 WRITE #1, "Alice", 28, 95.5
30 CLOSE #1
```
