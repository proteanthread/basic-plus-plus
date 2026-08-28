# `assert` Case-Insensitive Assertion Statement

## 1. BASIC Usage and Keyword Definition

Asserts that a logical condition is true, halting program execution with an error if it evaluates to false.

### Syntax Signatures:
```basic
ASSERT condition [, "failure_message"]
```

### Error Handling & Boundary Conditions:
- **Error 250 (ERR_ASSERTION_FAILED)**: Assertion failed.

### Operational Notes:
- Case-insensitive alias for CHECK statement.

---

## 2. Code Examples

```basic
10 ASSERT X >= 0, "X cannot be negative"
20 PRINT "Assertion passed."
```
