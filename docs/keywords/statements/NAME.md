# `NAME` Rename File Statement

## 1. BASIC Usage and Keyword Definition

Renames an existing file or directory on disk or in the virtual filesystem.

### Syntax Signatures:
```basic
NAME old_filename$ AS new_filename$
```

### Error Handling & Boundary Conditions:
- **Error 53 (ERR_FILE_NOT_FOUND)**: Source file does not exist.
- **Error 58 (ERR_FILE_ALREADY_EXISTS)**: Target file name already exists.

### Operational Notes:
- Cannot move files across distinct physical drives.

---

## 2. Code Examples

```basic
10 NAME "TEMP.DAT" AS "FINAL.DAT"
```
