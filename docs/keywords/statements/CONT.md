# `CONT` Continue Program Execution Statement

## 1. BASIC Usage and Keyword Definition

Resumes execution of a paused BASIC program following a STOP statement or interactive interrupt.

### Syntax Signatures:
```basic
CONT
```

### Error Handling & Boundary Conditions:
- **Error 17 (ERR_CANNOT_CONTINUE)**: Program modified or not in breakable state.

### Operational Notes:
- Restores instruction pointer and active loop stack frame.

---

## 2. Code Examples

```basic
10 PRINT "Step 1"
20 STOP
30 PRINT "Step 2 (resumed)"
```
