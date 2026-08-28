# `ENVIRONSTR` Environment String Function Alias

## 1. BASIC Usage and Function Definition

The `ENVIRONSTR` function is a keyword alias for `ENVIRON$`, returning an environment variable string by name or environment block index.

### Syntax Signatures:
```basic
val$ = ENVIRONSTR(name_string$)
val$ = ENVIRONSTR(index_number%)
```

### Operational Rules:
- Identical semantics and return values to `ENVIRON$`.

---

## 2. Code Examples

```basic
10 TMP$ = ENVIRONSTR("TEMP")
20 PRINT "Temporary directory: "; TMP$
```
