# `DIR` Directory File Listing Statement

## 1. BASIC Usage and Keyword Definition

Queries and lists directory contents matching an optional filename pattern.

### Syntax Signatures:
```basic
DIR [pattern$]
```

### Error Handling & Boundary Conditions:
- **Error 76 (ERR_PATH_NOT_FOUND)**: Directory not found.

### Operational Notes:
- Routes through VFS abstraction layer.

---

## 2. Code Examples

```basic
10 DIR "*.BAS"
20 DIR "SUBDIR/"
```
