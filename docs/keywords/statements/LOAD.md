# `LOAD` Program File Loading Statement

## 1. BASIC Usage and Keyword Definition

Loads a BASIC program file from disk into memory, optionally running it immediately if ', R' is appended.

### Syntax Signatures:
```basic
LOAD filename$ [, R]
```

### Error Handling & Boundary Conditions:
- **Error 53 (ERR_FILE_NOT_FOUND)**: Specified program file does not exist.

### Operational Notes:
- Clears current variable state unless CHAIN is used.

---

## 2. Code Examples

```basic
LOAD "GAME.BAS", R : REM Loads and immediately executes GAME.BAS
```
