# `GOSUB` Subroutine Execution Branch Statement

## 1. BASIC Usage and Keyword Definition

Pushes the return address onto the call stack and transfers execution to the designated line or label.

### Syntax Signatures:
```basic
GOSUB {line_number% | @label}
```

### Error Handling & Boundary Conditions:
- **Error 8 (ERR_UNDEFINED_LINE_NUMBER)**: Subroutine destination line not found.

### Operational Notes:
- Heap-managed return address stack prevents host stack overflow.

---

## 2. Code Examples

```basic
10 PRINT "Main program"
20 GOSUB 100
30 PRINT "Back in main"
40 END
100 PRINT "Inside subroutine"
110 RETURN
```
