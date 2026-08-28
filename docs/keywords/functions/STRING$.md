# `STRING$` Repeated Character String Generator Function

## 1. BASIC Usage and Function Definition

The `STRING$` function creates a string composed of a repeated character or ASCII byte code.

### Syntax Signatures:
```basic
res$ = STRING$(count%, ascii_code%)
res$ = STRING$(count%, char_str$)
```

### Operational Rules:
- Repeats character $N$ times. If $N \le 0$, returns empty string `""`.
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 PRINT STRING$(40, "=") : REM Outputs line of 40 '=' characters
20 PRINT STRING$(5, 65)   : REM Outputs "AAAAA"
```
