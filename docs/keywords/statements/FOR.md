# `FOR` Indexed Loop Initiation Statement

## 1. BASIC Usage and Keyword Definition

Executes a block of code repeatedly while incrementing or decrementing a counter variable.

### Syntax Signatures:
```basic
FOR variable = start_val TO end_val [STEP increment_val]
  [statements]
NEXT [variable]
```

### Operational Notes:
- Non-recursive loop stack maintained on VM heap.

---

## 2. Code Examples

```basic
10 FOR I = 1 TO 10 STEP 2
20   PRINT "I = "; I
30 NEXT I
```
