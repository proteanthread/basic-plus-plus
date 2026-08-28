# `DATE$` System Date String Function & Variable

## 1. BASIC Usage and Keyword Definition

The `DATE$` variable/function returns the current local system calendar date formatted as `"MM-DD-YYYY"`. In statement syntax, `DATE$ = "MM-DD-YYYY"` updates the system date.

### Syntax Signatures:
```basic
cur_date$ = DATE$
DATE$ = "08-15-2026"
```

### Operational Rules:
- Returns a 10-character string in `MM-DD-YYYY` format.
- Assignment requires system capability permissions (`CAP_SYS`).

---

## 2. Code Examples

```basic
10 PRINT "Current System Date: "; DATE$
```
