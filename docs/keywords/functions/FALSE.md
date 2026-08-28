# `FALSE` Boolean False Constant

## 1. BASIC Usage and Keyword Definition

The `FALSE` keyword is a built-in constant evaluating to numeric `0`, representing the boolean false truth value.

### Syntax Signatures:
```basic
flag% = FALSE
```

### Operational Rules:
- Evaluates to integer `0`.
- In logical expressions, `NOT FALSE` evaluates to `TRUE` (`-1`).

---

## 2. Code Examples

```basic
10 LET IS_RUNNING% = FALSE
20 IF IS_RUNNING% = FALSE THEN PRINT "Engine is currently stopped."
```
