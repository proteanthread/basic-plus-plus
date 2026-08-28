# `SAVE` Save Program File Statement

## 1. BASIC Usage and Keyword Definition

Saves the BASIC program in memory to a file on disk in ASCII text (A), tokenized binary (B), or protected (P) format.

### Syntax Signatures:
```basic
SAVE filename$ [, {A | B | P}]
```

### Error Handling & Boundary Conditions:
- **Error 61 (ERR_DISK_FULL)**: Insufficient storage space on disk.

### Operational Notes:
- Defaults to standard portable UTF-8 ASCII format.

---

## 2. Code Examples

```basic
SAVE "GAME.BAS", A : REM Saves as plaintext ASCII file
```
