# `ON_KEY` Function Key Event Trapping Statement

## 1. BASIC Usage and Keyword Definition

Installs an event handler subroutine for programmable function keys (F1-F10).

### Syntax Signatures:
```basic
ON KEY(key_num%) GOSUB {line_number% | @label}
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Key number outside range (1..10).

### Operational Notes:
- Requires KEY(n) ON to activate trap.

---

## 2. Code Examples

```basic
10 ON KEY(1) GOSUB 100 : REM F1 handler
20 KEY(1) ON
30 DO : LOOP
100 PRINT "F1 Help requested": RETURN
```
