# `CURDIRDOLLAR` Current Directory Path Function

## 1. BASIC Usage and Keyword Definition

Returns the canonical absolute path of the current working directory.

### Syntax Signatures:
```basic
path$ = CURDIR$([drive$])
```

### Error Handling & Boundary Conditions:
- **Error 68 (ERR_DEVICE_UNAVAILABLE)**: Specified drive does not exist.

### Operational Notes:
- Returns string with reference counting semantics.

---

## 2. Code Examples

```basic
10 PRINT "Working Directory: "; CURDIR$
```
