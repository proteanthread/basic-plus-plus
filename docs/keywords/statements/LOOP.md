# `LOOP` Structured Loop Termination Statement

## 1. BASIC Usage and Keyword Definition

Marks the end of a DO structured loop block, optionally testing a continuation or exit condition.

### Syntax Signatures:
```basic
LOOP [{WHILE | UNTIL} condition]
```

### Operational Notes:
- Pairs with DO statement.

---

## 2. Code Examples

```basic
10 DO
20   X = X + 1
30 LOOP UNTIL X >= 10
```
