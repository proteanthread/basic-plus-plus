# `END` Program Termination Statement

## 1. BASIC Usage and Keyword Definition

Cleanly terminates program execution, closes all open files, and returns to the system prompt.

### Syntax Signatures:
```basic
END
```

### Operational Notes:
- Resets VM execution state without clearing stored program lines.

---

## 2. Code Examples

```basic
10 PRINT "Starting process..."
20 END
30 PRINT "This line never executes"
```
