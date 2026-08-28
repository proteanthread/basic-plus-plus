# `RETURN` Return from GOSUB Subroutine Statement

## 1. BASIC Usage and Keyword Definition

Pops the return address from the call stack and transfers execution back to the statement following GOSUB.

### Syntax Signatures:
```basic
RETURN [{line_number% | @label}]
```

### Error Handling & Boundary Conditions:
- **Error 3 (ERR_RETURN_WITHOUT_GOSUB)**: No GOSUB active on call stack.

### Operational Notes:
- Optional target line performs non-local subroutine exit.

---

## 2. Code Examples

```basic
10 GOSUB 100
20 PRINT "Returned to line 20"
30 END
100 PRINT "Inside sub"
110 RETURN
```
