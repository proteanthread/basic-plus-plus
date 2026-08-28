# `DELETE` Program Line Deletion Command

## 1. BASIC Usage and Keyword Definition

Deletes a single program line or a contiguous range of lines from active program memory.

### Syntax Signatures:
```basic
DELETE [start_line%] - [end_line%]
DELETE line%
```

### Error Handling & Boundary Conditions:
- **Error 8 (ERR_UNDEFINED_LINE_NUMBER)**: Specified line range not found.

### Operational Notes:
- Silent execution in non-interactive batch runners.

---

## 2. Code Examples

```basic
10 REM Line to keep
20 REM Line to delete
30 REM Line to delete
DELETE 20-30
```
