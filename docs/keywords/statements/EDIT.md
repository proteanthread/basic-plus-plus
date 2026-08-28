# `EDIT` Program Line Editor Command

## 1. BASIC Usage and Keyword Definition

Loads the specified program line into the console input buffer for interactive in-place editing.

### Syntax Signatures:
```basic
EDIT line_number%
```

### Error Handling & Boundary Conditions:
- **Error 8 (ERR_UNDEFINED_LINE_NUMBER)**: Line does not exist.

### Operational Notes:
- Supported in interactive REPL environments.

---

## 2. Code Examples

```basic
10 PRINT "Misspelled line"
EDIT 10
```
