# `USERNAME$` Active User Name Function

## 1. BASIC Usage and Function Definition

The `USERNAME$` function returns the username of the currently logged-in operating system user from `libplatform`.

### Syntax Signatures:
```basic
user$ = USERNAME$
user$ = USERNAME$()
```

### Operational Rules:
- Returns the OS account name string retrieved via platform abstractions.

---

## 2. Code Examples

```basic
10 PRINT "Welcome back, "; USERNAME$; "!"
```
