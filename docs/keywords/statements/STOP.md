# `STOP` Program Execution Breakpoint Statement

## 1. BASIC Usage and Keyword Definition

Suspends program execution, displays a break message with line number, and returns to interactive prompt mode.

### Syntax Signatures:
```basic
STOP
```

### Operational Notes:
- Execution can be resumed at next line via CONT command.

---

## 2. Code Examples

```basic
10 X = 100
20 STOP : REM Breakpoint: Examine X
30 PRINT "Resumed: "; X
```
