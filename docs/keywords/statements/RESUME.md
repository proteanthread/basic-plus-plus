# `RESUME` Resume Execution After Error Handler Statement

## 1. BASIC Usage and Keyword Definition

Exits an active ON ERROR GOTO handler, clearing the error state and resuming program execution.

### Syntax Signatures:
```basic
RESUME [0 | NEXT | {line_number% | @label}]
```

### Error Handling & Boundary Conditions:
- **Error 20 (ERR_RESUME_WITHOUT_ERROR)**: RESUME executed outside active error handler.

### Operational Notes:
- RESUME 0 retries failing statement; RESUME NEXT continues at next statement.

---

## 2. Code Examples

```basic
10 ON ERROR GOTO 100
20 ERROR 6 : REM Force overflow
30 PRINT "Execution resumed successfully."
40 END
100 PRINT "Handling error #"; ERR
110 RESUME NEXT
```
