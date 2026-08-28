# `MAX` Maximum Value Function (Universal Dual-Format)

## 1. BASIC Usage and Function/Operator Definition

The `MAX` keyword evaluates numeric expressions and returns the largest (maximum) numeric value. It is supported in both **infix** notation (`a MAX b`) and **prefix/variadic** functional notation (`MAX(a, b [, c...])`).

### Syntax Signatures:
```basic
largest = val1 MAX val2
largest = MAX(val1, val2 [, val3, ...])
```

### Operational Rules:
- Returns $\max(x_1, x_2, \dots, x_n)$.
- In RPN stack evaluation blocks: `{ 10 20 MAX }`.

---

## 2. Code Examples

```basic
10 PRINT "Infix: "; 42 MAX 17 : REM Outputs 42
20 PRINT "Prefix: "; MAX(42, 17, 88, 5) : REM Outputs 88
```
