# `IF_POSTFIX` Trailing Conditional Statement Modifier

## 1. BASIC Usage and Keyword Definition

Executes a preceding statement only if the trailing condition expression evaluates to true.

### Syntax Signatures:
```basic
statement IF condition
```

### Operational Notes:
- Multiple trailing postfix conditions evaluate right-to-left.

---

## 2. Code Examples

```basic
10 PRINT "X is positive" IF X > 0
20 LET Y = 10 IF Flag% = 1
```
