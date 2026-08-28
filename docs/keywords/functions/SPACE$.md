# `SPACE$` Space String Generator Function

## 1. BASIC Usage and Function Definition

The `SPACE$` function generates a string consisting of a specified number of consecutive ASCII space characters (`CHR$(32)`).

### Syntax Signatures:
```basic
spaces$ = SPACE$(count%)
```

### Operational Rules:
- If `count% <= 0`, returns empty string `""`.
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 PRINT "Col1"; SPACE$(10); "Col2"
```
