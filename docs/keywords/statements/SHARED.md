# `SHARED` Procedure Variable Scope Statement

## 1. BASIC Usage and Keyword Definition

The `SHARED` statement grants a `SUB` procedure or `FUNCTION` block direct access to module-level (global) variables without passing them as parameters.

### Syntax Signatures:
```basic
SHARED var1 [, var2, var3, ...]
DIM SHARED var1(...) [AS type]
```

### Operational Rules:
- Used inside `SUB ... END SUB` or `FUNCTION ... END FUNCTION` bodies.
- Binds specified identifiers to the module-level variable scope rather than creating procedure-local variables.

---

## 2. Code Examples

```basic
10 GLOBAL_COUNTER = 0
20 CALL IncrementCounter()
30 CALL IncrementCounter()
40 PRINT "Final counter value: "; GLOBAL_COUNTER
50 END

100 SUB IncrementCounter()
110   SHARED GLOBAL_COUNTER
120   GLOBAL_COUNTER = GLOBAL_COUNTER + 1
130 END SUB
```
