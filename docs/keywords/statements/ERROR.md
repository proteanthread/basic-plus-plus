# `ERROR` Runtime Error Simulation Statement

## 1. BASIC Usage and Keyword Definition

Simulates a runtime error with the specified numeric code, triggering active ON ERROR traps.

### Syntax Signatures:
```basic
ERROR error_code%
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Error code < 1 or > 255.

### Operational Notes:
- Enables custom error code dispatching and unit test validation.

---

## 2. Code Examples

```basic
10 ON ERROR GOTO 100
20 ERROR 55 : REM File already open
30 END
100 PRINT "Simulated error: "; ERR
110 RESUME NEXT
```
