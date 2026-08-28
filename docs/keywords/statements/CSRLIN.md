# `CSRLIN` Cursor Row Retrieval System Variable

## 1. BASIC Usage and Keyword Definition

Returns the 1-based vertical row coordinate of the text cursor on the active display.

### Syntax Signatures:
```basic
row% = CSRLIN
```

### Operational Notes:
- Authoritative cursor row coordinate read from virtual console state.

---

## 2. Code Examples

```basic
10 CLS
20 LOCATE 5, 10
30 PRINT "Cursor is at row: "; CSRLIN : REM Outputs 5
```
