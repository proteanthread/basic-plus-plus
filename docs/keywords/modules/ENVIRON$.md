# `ENVIRON$` Environment Variable Query Function

## 1. BASIC Usage and Function Definition

The `ENVIRON$` function returns the value of an environment variable as a string, queried either by variable name or by 1-based index in the environment block.

### Syntax Signatures:
```basic
val$ = ENVIRON$(name_string$)
val$ = ENVIRON$(index_number%)
```

### Operational Rules:
- **Query by Name**: `ENVIRON$("PATH")` returns the value string of the named variable (or empty string `""` if not found).
- **Query by Index**: `ENVIRON$(1)` returns the entire `"NAME=value"` entry at the specified index position.
- **Reference-Counted String**: Returns `VAL_STRING` managed by the string pool.

---

## 2. Code Examples

```basic
10 USER$ = ENVIRON$("USERNAME")
20 IF USER$ = "" THEN USER$ = ENVIRON$("USER")
30 PRINT "Current user: "; USER$
```
