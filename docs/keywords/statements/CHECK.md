# `CHECK` Assertion and Invariant Check Statement

## 1. BASIC Usage and Keyword Definition

Validates a logical assertion, terminating execution with an error if the condition evaluates to false.

### Syntax Signatures:
```basic
CHECK condition [, "failure_message"]
```

### Error Handling & Boundary Conditions:
- **Error 250 (ERR_ASSERTION_FAILED)**: Evaluated condition was false (0).

### Operational Notes:
- Zero runtime overhead when assertions are disabled via compiler switches.

---

## 2. Code Examples

```basic
10 X = 100
20 CHECK X > 0, "X must be positive"
30 PRINT "Check passed."
```
