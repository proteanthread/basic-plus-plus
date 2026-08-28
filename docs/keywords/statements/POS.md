# `POS` Cursor Horizontal Column Query Function

## 1. BASIC Usage and Keyword Definition

Returns the current 1-based horizontal column position of the text cursor.

### Syntax Signatures:
```basic
col% = POS(dummy%)
```

### Operational Notes:
- Returns active column offset (1..WIDTH).

---

## 2. Code Examples

```basic
10 PRINT "Hello";
20 PRINT " (Cursor was at col: "; POS(0); ")"
```
