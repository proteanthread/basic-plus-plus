# `ERR` Error Code System Variable

## 1. BASIC Usage and Keyword Definition

Returns the numeric error code of the most recent runtime error.

### Syntax Signatures:
```basic
code% = ERR
```

### Operational Notes:
- Reset to 0 upon executing RESUME.

---

## 2. Code Examples

```basic
10 ON ERROR GOTO 100
20 ERROR 6 : REM Force overflow
30 END
100 PRINT "Trapped error code: "; ERR
110 RESUME NEXT
```
