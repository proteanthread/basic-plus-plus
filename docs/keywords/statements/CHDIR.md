# `CHDIR` Change Working Directory Statement

## 1. BASIC Usage and Keyword Definition

Changes the active working directory for file resolution in the process and VFS.

### Syntax Signatures:
```basic
CHDIR path_expression$
```

### Error Handling & Boundary Conditions:
- **Error 76 (ERR_PATH_NOT_FOUND)**: Directory path does not exist.
- **Error 70 (ERR_PERMISSION_DENIED)**: Security sandbox violation.

### Operational Notes:
- Changes process working directory and VFS context path.

---

## 2. Code Examples

```basic
10 CHDIR "DATA_FILES"
20 FILES
30 CHDIR ".."
```
