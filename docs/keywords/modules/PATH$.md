# `PATH$` File Path Resolution Function

## 1. BASIC Usage and Function Definition

The `PATH$` function resolves a relative file or directory path against the currently executing module or working directory, returning a normalized absolute path string.

### Syntax Signatures:
```basic
full_path$ = PATH$(relative_path$)
```

### Operational Rules:
- Normalizes path separators and removes redundant `/./` or `/../` segments.
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 CONF$ = PATH$("config/settings.ini")
20 PRINT "Resolved Config Path: "; CONF$
```
