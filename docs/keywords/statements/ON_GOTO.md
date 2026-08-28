# `ON_GOTO` Computed Jump Branch Statement

## 1. BASIC Usage and Keyword Definition

Evaluates an integer expression N and jumps unconditionally to the N-th line number in the list.

### Syntax Signatures:
```basic
ON expression GOTO line1% [, line2% [, line3%...]]
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Expression evaluates to < 0 or > 255.

### Operational Notes:
- Drops through if N is 0 or > count.

---

## 2. Code Examples

```basic
10 INPUT "Option (1-3): ", OPT%
20 ON OPT% GOTO 100, 200, 300
30 PRINT "Invalid option": GOTO 10
100 PRINT "Option 1 Selected": END
200 PRINT "Option 2 Selected": END
300 PRINT "Option 3 Selected": END
```
