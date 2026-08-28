# `DO` Structured Loop Initiation Statement

## 1. BASIC Usage and Keyword Definition

Initiates a structured loop block with optional entry or exit test conditions.

### Syntax Signatures:
```basic
DO [{WHILE | UNTIL} condition]
  [statements]
LOOP [{WHILE | UNTIL} condition]
```

### Operational Notes:
- Non-recursive VM loop state tracked on heap control stack.

---

## 2. Code Examples

```basic
10 I = 1
20 DO WHILE I <= 5
30   PRINT "Count: "; I
40   I = I + 1
50 LOOP
```
