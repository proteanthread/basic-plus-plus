# `ON_GOSUB` Computed Subroutine Branch Statement

## 1. BASIC Usage and Keyword Definition

Evaluates an integer expression N and executes a GOSUB branch to the N-th line number in the list.

### Syntax Signatures:
```basic
ON expression GOSUB line1% [, line2% [, line3%...]]
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Expression evaluates to < 0 or > 255.

### Operational Notes:
- If index is 0 or exceeds list count, execution drops through to next statement.

---

## 2. Code Examples

```basic
10 CHOICE% = 2
20 ON CHOICE% GOSUB 100, 200, 300
30 END
100 PRINT "Choice 1": RETURN
200 PRINT "Choice 2": RETURN
300 PRINT "Choice 3": RETURN
```
