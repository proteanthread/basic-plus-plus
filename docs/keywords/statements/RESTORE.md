# `RESTORE` Reset DATA Statement Pointer Statement

## 1. BASIC Usage and Keyword Definition

Resets the internal DATA pointer to the beginning of the program, or to the specified line number/label.

### Syntax Signatures:
```basic
RESTORE [line_number% | @label]
```

### Error Handling & Boundary Conditions:
- **Error 8 (ERR_UNDEFINED_LINE_NUMBER)**: Target line not found.

### Operational Notes:
- Allows multiple passes over embedded data tables.

---

## 2. Code Examples

```basic
10 READ A, B
20 RESTORE : REM Reset back to line 40
30 READ C, D
40 DATA 10, 20
```
