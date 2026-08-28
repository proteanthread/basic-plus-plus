# `RENAME` Rename File Command Alias

## 1. BASIC Usage and Keyword Definition

Alias for the standard NAME statement, renaming an existing file on disk.

### Syntax Signatures:
```basic
RENAME old_filename$ TO new_filename$
```

### Error Handling & Boundary Conditions:
- **Error 53 (ERR_FILE_NOT_FOUND)**: File does not exist.

### Operational Notes:
- Convenience alias for NAME...AS.

---

## 2. Code Examples

```basic
10 RENAME "OLD.BAS" TO "NEW.BAS"
```
