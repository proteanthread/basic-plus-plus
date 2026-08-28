# `RMDIR` Remove Directory Statement

## 1. BASIC Usage and Keyword Definition

Removes an existing empty directory from disk or the virtual filesystem.

### Syntax Signatures:
```basic
RMDIR path_expression$
```

### Error Handling & Boundary Conditions:
- **Error 76 (ERR_PATH_NOT_FOUND)**: Directory does not exist.
- **Error 75 (ERR_PATH_FILE_ACCESS_ERROR)**: Directory not empty or access denied.

### Operational Notes:
- Directory must be empty before removal.

---

## 2. Code Examples

```basic
10 RMDIR "TEMP_DIR"
```
