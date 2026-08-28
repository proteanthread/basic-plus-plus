# `UNLESS` Inverse Conditional Statement Modifier

## 1. BASIC Usage and Keyword Definition

Executes a preceding statement only if the trailing condition expression evaluates to false (0).

### Syntax Signatures:
```basic
statement UNLESS condition
```

### Operational Notes:
- Syntactic sugar for IF NOT (condition).

---

## 2. Code Examples

```basic
10 PRINT "System operational" UNLESS ErrorFlag% = 1
```
