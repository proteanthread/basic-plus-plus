# `FILES` Filesystem Directory Listing Statement

## 1. BASIC Usage and Keyword Definition

Displays a formatted list of files and directories on the active screen matching an optional wildcard.

### Syntax Signatures:
```basic
FILES [pattern$]
```

### Error Handling & Boundary Conditions:
- **Error 76 (ERR_PATH_NOT_FOUND)**: Path not found.

### Operational Notes:
- Standard GW-BASIC directory listing command.

---

## 2. Code Examples

```basic
10 FILES "*.DAT"
20 FILES "C:\GAMES\*.BAS"
```
