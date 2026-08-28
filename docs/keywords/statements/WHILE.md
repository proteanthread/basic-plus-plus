# `WHILE` Structured While Loop Initiation Statement

## 1. BASIC Usage and Keyword Definition

Repeatedly executes a block of statements as long as the entry condition evaluates to true (non-zero).

### Syntax Signatures:
```basic
WHILE condition
  [statements]
WEND
```

### Operational Notes:
- Non-recursive VM loop frame evaluation.

---

## 2. Code Examples

```basic
10 WHILE NOT EOF(1)
20   LINE INPUT #1, L$
30   PRINT L$
40 WEND
```
