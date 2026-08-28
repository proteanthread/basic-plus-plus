# `RUN` Execute Program Statement

## 1. BASIC Usage and Keyword Definition

Clears all variables and starts execution of the program in memory or loads and executes a program file from disk.

### Syntax Signatures:
```basic
RUN [line_number% | filename$ [, R]]
```

### Error Handling & Boundary Conditions:
- **Error 53 (ERR_FILE_NOT_FOUND)**: Program file does not exist.

### Operational Notes:
- Appends ', R' to preserve open file channels across programs.

---

## 2. Code Examples

```basic
RUN           : REM Runs program in memory
RUN "MENU.BAS" : REM Loads and runs MENU.BAS
```
