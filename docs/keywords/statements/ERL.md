# `ERL` Error Line Number System Variable

## 1. BASIC Usage and Keyword Definition

Returns the program line number where the most recent runtime error occurred.

### Syntax Signatures:
```basic
line% = ERL
```

### Operational Notes:
- Returns 0 if the error occurred in direct/immediate mode.

---

## 2. Code Examples

```basic
10 ON ERROR GOTO 100
20 ERROR 11 : REM Force division by zero
30 END
100 PRINT "Error occurred on line: "; ERL
110 RESUME NEXT
```
