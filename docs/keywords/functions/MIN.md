# `MIN` Minimum Value Function (Universal Dual-Format)

## 1. BASIC Usage and Function/Operator Definition

The `MIN` keyword evaluates numeric expressions and returns the smallest (minimum) numeric value. It is supported in both **infix** notation (`a MIN b`) and **prefix/variadic** functional notation (`MIN(a, b [, c...])`).

### Syntax Signatures:
```basic
smallest = val1 MIN val2
smallest = MIN(val1, val2 [, val3, ...])
```

### Operational Rules:
- Returns $\min(x_1, x_2, \dots, x_n)$.
- In RPN stack evaluation blocks: `{ 10 20 MIN }`.

---

## 2. Code Examples

```basic
10 PRINT "Infix: "; 42 MIN 17 : REM Outputs 17
20 PRINT "Prefix: "; MIN(42, 17, 88, 5) : REM Outputs 5
```
