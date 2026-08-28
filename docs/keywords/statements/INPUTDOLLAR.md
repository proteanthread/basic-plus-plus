# `INPUTDOLLAR` Fixed Length Character Stream Reader Function

## 1. BASIC Usage and Keyword Definition

Reads a specified number of raw characters from the keyboard buffer or from an open file channel.

### Syntax Signatures:
```basic
chars$ = INPUT$(byte_count% [, [#]filenum%])
```

### Error Handling & Boundary Conditions:
- **Error 52 (ERR_BAD_FILE_NUMBER)**: File channel is invalid.

### Operational Notes:
- Preserves binary character values including nulls and control bytes.

---

## 2. Code Examples

```basic
10 OPEN "B", #1, "DATA.BIN"
20 HEADER$ = INPUT$(4, #1)
30 CLOSE #1
40 PRINT "Magic Header: "; HEADER$
```
