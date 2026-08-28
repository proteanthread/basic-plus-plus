# `ON_ERROR_GOTO` Error Trapping Vector Statement

## 1. BASIC Usage and Keyword Definition

Installs an error trapping handler. Specifying 0 disables error trapping and restores default error termination.

### Syntax Signatures:
```basic
ON ERROR GOTO {line_number% | @label | 0}
```

### Operational Notes:
- Essential for robust fault-tolerant BASIC programming.

---

## 2. Code Examples

```basic
10 ON ERROR GOTO 100
20 OPEN "I", #1, "NONEXISTENT.FILE"
30 END
100 PRINT "Caught error #"; ERR; " on line "; ERL
110 RESUME NEXT
```
