# `REM` Remark / Comment Statement

## 1. BASIC Usage and Keyword Definition

The `REM` statement (and shorthand single quote `'`) allows inserting explanatory remarks, comments, and documentation notes into a program. The interpreter ignores all text following `REM` or `'` until the end of the physical line.

### Syntax Signatures:
```basic
REM comment_text
' comment_text
statement : REM comment_text
statement ' comment_text
```

### Operational Rules:
- The entire rest of the line following `REM` or `'` is treated as comment text and ignored by the AST evaluator.
- Unlike other statements, colons (`:`) occurring inside a `REM` line are treated as literal text, not statement separators.

---

## 2. Code Examples

```basic
10 REM ===================================================
20 REM Program: Inventory Manager v1.0
30 REM ===================================================
40 LET ITEMS = 50 ' Initial stock count
50 PRINT "Items in stock:"; ITEMS
```
