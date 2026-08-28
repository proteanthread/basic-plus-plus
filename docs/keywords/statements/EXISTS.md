# `EXISTS` File Existence Query Function

## 1. BASIC Usage and Keyword Definition

Returns -1 (TRUE) if the specified file or directory path exists on disk, or 0 (FALSE) otherwise.

### Syntax Signatures:
```basic
is_file% = EXISTS(filename$)
```

### Operational Notes:
- Safe existence check that does not throw filesystem error exceptions.

---

## 2. Code Examples

```basic
10 IF EXISTS("CONFIG.DAT") THEN
20   PRINT "Config file present."
30 ELSE
40   PRINT "Config missing!"
50 END IF
```
