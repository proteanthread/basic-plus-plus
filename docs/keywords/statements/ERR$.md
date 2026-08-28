# `ERR$` Error Message Description Function

## 1. BASIC Usage and Keyword Definition

Returns the human-readable text description corresponding to a runtime error code.

### Syntax Signatures:
```basic
message$ = ERR$([error_code%])
```

### Operational Notes:
- If omitted, error_code defaults to the current value of ERR.

---

## 2. Code Examples

```basic
10 PRINT "Error 5 is: "; ERR$(5) : REM Outputs "Illegal function call"
```
