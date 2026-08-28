# `MKDIR` Make Directory Statement

## 1. BASIC Usage and Keyword Definition

Creates a new directory on disk or in the virtual filesystem.

### Syntax Signatures:
```basic
MKDIR path_expression$
```

### Error Handling & Boundary Conditions:
- **Error 75 (ERR_PATH_FILE_ACCESS_ERROR)**: Directory already exists or parent missing.

### Operational Notes:
- Requires CAP_FS filesystem write permissions.

---

## 2. Code Examples

```basic
10 MKDIR "BACKUPS"
20 MKDIR "LOGS/2026"
```
