# `ON_TIMER` Timer Interval Event Trapping Statement

## 1. BASIC Usage and Keyword Definition

Installs an event handler that triggers periodically at the specified elapsed second interval.

### Syntax Signatures:
```basic
ON TIMER(seconds%) GOSUB {line_number% | @label}
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Interval <= 0 or > 86400.

### Operational Notes:
- Requires TIMER ON to activate.

---

## 2. Code Examples

```basic
10 ON TIMER(1) GOSUB 100 : REM Every 1 second
20 TIMER ON
30 DO : LOOP
100 PRINT "Tick: "; TIME$ : RETURN
```
